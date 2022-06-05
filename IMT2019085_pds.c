#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "pds.h"
#include "bst.h"

struct PDS_RepoInfo repo_handle;

void preorder(struct BST_Node *root)
{
  if (root == NULL)
    return;

  struct PDS_NdxInfo *data_obj = (struct PDS_NdxInfo *)root->data;
  if (data_obj->is_deleted == 0)
    fwrite(root->data, sizeof(struct PDS_NdxInfo), 1, repo_handle.pds_ndx_fp);

  preorder(root->left_child);
  preorder(root->right_child);
}
int pds_create(char *repo_name) // Small changes to accomodate running test cases again
{
  char filename[30], indexfile[30];
  strcpy(filename, repo_name);
  strcpy(indexfile, repo_name);
  strcat(filename, ".dat");
  strcat(indexfile, ".ndx");
  FILE *fp = fopen(filename, "wb+");
  FILE *ifp = fopen(indexfile, "wb+");
  if (fp == NULL || ifp == NULL)
    return PDS_FILE_ERROR;
  fclose(fp);
  fclose(ifp);

  return PDS_SUCCESS;
}

int pds_open(char *repo_name, int rec_size) // Same as before
{
  char *datfilename = malloc(strlen(repo_name) + 1);
  strcpy(datfilename, repo_name);
  char *ndxfilename = malloc(strlen(repo_name) + 1);
  strcpy(ndxfilename, repo_name);

  strcat(datfilename, ".dat");
  strcat(ndxfilename, ".ndx");

  FILE *dfp = fopen(datfilename, "rb+");
  FILE *ndxfp = fopen(ndxfilename, "ab+");

  if (repo_handle.repo_status == PDS_REPO_OPEN)
    return PDS_REPO_ALREADY_OPEN;
  if (!dfp || !ndxfp)
  {
    return PDS_FILE_ERROR;
  }
  strcpy(repo_handle.pds_name, repo_name);
  repo_handle.pds_data_fp = dfp;
  repo_handle.pds_ndx_fp = ndxfp;
  repo_handle.rec_size = rec_size;
  repo_handle.repo_status = PDS_REPO_OPEN;

  if (pds_load_ndx() == PDS_LOAD_NDX_FAILED)
    return PDS_LOAD_NDX_FAILED;

  fclose(repo_handle.pds_ndx_fp);
  // Open the data file and index file in rb+ mode
  // Update the fields of PDS_RepoInfo appropriately
  // Build BST and store in pds_bst by reading index entries from the index file
  // Close only the index file
}

int pds_load_ndx() // Same as before
{
  repo_handle.pds_bst = NULL;
  struct PDS_NdxInfo *ndxobj = (struct PDS_NdxInfo *)malloc(sizeof(struct PDS_NdxInfo));

  while (fread(ndxobj, sizeof(struct PDS_NdxInfo), 1, repo_handle.pds_ndx_fp) == 1)
  {

    if (bst_add_node(&repo_handle.pds_bst, ndxobj->key, ndxobj) != BST_SUCCESS)
      return PDS_LOAD_NDX_FAILED;
    ndxobj = (struct PDS_NdxInfo *)malloc(sizeof(struct PDS_NdxInfo));
  }
  return BST_SUCCESS;
  // Internal function used by pds_open to read index entries into BST
}

int put_rec_by_key(int key, void *rec) // Same as before
{
  if (repo_handle.repo_status == PDS_REPO_OPEN)
  {
    fseek(repo_handle.pds_data_fp, 0, SEEK_END);
    struct PDS_NdxInfo *ndxobj = (struct PDS_NdxInfo *)malloc(sizeof(struct PDS_NdxInfo));
    ndxobj->key = key;
    ndxobj->offset = ftell(repo_handle.pds_data_fp);
    ndxobj->is_deleted = 0;

    int status = bst_add_node(&repo_handle.pds_bst, ndxobj->key, ndxobj);
    if (status == BST_SUCCESS)
    {
      if (fwrite(&key, sizeof(int), 1, repo_handle.pds_data_fp) == 1)
      {
        if (fwrite(rec, repo_handle.rec_size, 1, repo_handle.pds_data_fp) == 1)
          return PDS_SUCCESS;
      }
    }
    else
      return status;
  }
  return PDS_ADD_FAILED;
  // Seek to the end of the data file
  // Create an index entry with the current data file location using ftell
  // Add index entry to BST using offset returned by ftell
  // Write the key at the current data file location
  // Write the record after writing the key
}

int get_rec_by_ndx_key(int key, void *rec) // Only Function Rename
{
  struct BST_Node *data_node = bst_search(repo_handle.pds_bst, key);

  if (data_node)
  {
    struct PDS_NdxInfo *data_obj = (struct PDS_NdxInfo *)data_node->data;
    int search_offset = data_obj->offset;
    if (data_obj->is_deleted == 1)
      return PDS_REC_NOT_FOUND;

    if (repo_handle.repo_status == PDS_REPO_OPEN)
    {
      fseek(repo_handle.pds_data_fp, search_offset, SEEK_SET);

      int file_key;

      if (fread(&file_key, sizeof(int), 1, repo_handle.pds_data_fp) == 1)
      {
        if (fread(rec, repo_handle.rec_size, 1, repo_handle.pds_data_fp) == 1)
        {
          if (file_key == key)
          {
            return PDS_SUCCESS;
          }
        }
      }
      return PDS_REC_NOT_FOUND;
    }
    return PDS_REPO_CLOSED;
  }
  return PDS_REC_NOT_FOUND;
  // Search for index entry in BST
  // Seek to the file location based on offset in index entry
  // Read the key at the current file location
  // Read the record after reading the key
}

int pds_close() // Same as before
{
  if (repo_handle.repo_status != PDS_REPO_OPEN)
    return PDS_FILE_ERROR;
  char *ndxfilename = malloc(strlen(repo_handle.pds_name) + 1);
  strcpy(ndxfilename, repo_handle.pds_name);
  strcat(ndxfilename, ".ndx");
  repo_handle.pds_ndx_fp = fopen(ndxfilename, "wb");

  preorder(repo_handle.pds_bst);
  bst_destroy(repo_handle.pds_bst);

  fclose(repo_handle.pds_ndx_fp);
  fclose(repo_handle.pds_data_fp);
  repo_handle.repo_status = PDS_REPO_CLOSED;

  return PDS_SUCCESS;
  // Open the index file in wb mode (write mode, not append mode)
  // Unload the BST into the index file by traversing it in PRE-ORDER (overwrite the entire index file)
  // Free the BST by calling bst_destroy()
  // Close the index file and data file
}

int get_rec_by_non_ndx_key(void *key, void *rec, int (*matcher)(void *rec, void *key), int *io_count)
{

  if (repo_handle.repo_status == PDS_REPO_OPEN)
  {
    fseek(repo_handle.pds_data_fp, 0, SEEK_SET);
    int file_key;

    while (fread(&file_key, sizeof(int), 1, repo_handle.pds_data_fp) == 1)
    {
      if (fread(rec, repo_handle.rec_size, 1, repo_handle.pds_data_fp) == 1)
      {
        
        
        // if not deleted check matcher
        *io_count += 1;
        if (matcher(rec, key) == 0)
        {
          //checking if deleted.
          struct BST_Node *data_node = bst_search(repo_handle.pds_bst, file_key);
          if (data_node)
          {
            struct PDS_NdxInfo *data_obj = (struct PDS_NdxInfo *)data_node->data;
            if (data_obj->is_deleted == 1)
              return PDS_REC_NOT_FOUND;
          }
          return PDS_SUCCESS;
        }
      }
    }
    return PDS_REC_NOT_FOUND;
  }
  return PDS_REPO_CLOSED;
  // Seek to beginning of file
  // Perform a table scan - iterate over all the records
  //   Read the key and the record
  //   Increment io_count by 1 to reflect count no. of records read
  //   Use the function in function pointer to compare the record with required key
  // Return success when record is found
}
int delete_rec_by_ndx_key(int key) // New Function
{
  struct BST_Node *data_node = bst_search(repo_handle.pds_bst, key);

  if (data_node)
  {
    struct PDS_NdxInfo *data_obj = (struct PDS_NdxInfo *)data_node->data;
    if (data_obj->is_deleted == 0)
    {
      data_obj->is_deleted = 1;
      return PDS_SUCCESS;
    }
  }

  return PDS_DELETE_FAILED;
  // Search for the record in the BST using the key
  // If record not found, return PDS_DELETE_FAILED
  // If record is found, check if it has already been deleted, if so return PDS_DELETE_FAILED
  // Else, set the record to deleted and return PDS_SUCCESS
}