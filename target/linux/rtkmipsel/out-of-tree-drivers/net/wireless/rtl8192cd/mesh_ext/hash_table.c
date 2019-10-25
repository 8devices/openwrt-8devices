#ifdef __KERNEL__
#include <linux/slab.h>
#else
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#endif
#include "hash_table.h"

long remove_hash_table (struct hash_table* tbl)
{
	unsigned long	i = 0;
	long 		ret = HASH_TABLE_FAILED;
	
	for(i = 0; i < (1 << tbl->table_size_power); i++)
	{
#ifdef __KERNEL__
		kfree(tbl->entry_array[i].hash_id);
		kfree(tbl->entry_array[i].data);
#else
		free(tbl->entry_array[i].hash_id);
		free(tbl->entry_array[i].data);
#endif
	}
#ifdef __KERNEL__
	kfree(tbl->entry_array);
#else
	free(tbl->entry_array);
#endif
	ret = HASH_TABLE_SUCCEEDED;
	
	return ret;
}

long init_hash_table (struct hash_table* tbl, unsigned long tbl_sz_pwr, unsigned long hid_len, unsigned long data_len, void* hash_func, void* search_func, void* insert_func, void* delete_func, void* traverse_func)
{
	long ret = HASH_TABLE_FAILED;
	unsigned long i, tbl_sz;

	tbl->table_size_power = tbl_sz_pwr;
	tbl_sz = 1 << tbl->table_size_power;	
	tbl->hash_id_length = hid_len;
	tbl->data_length = data_len;

#ifdef __KERNEL__
	tbl->entry_array = (struct hash_table_entry*)kmalloc(sizeof(struct hash_table_entry) * tbl_sz, GFP_ATOMIC);
#else
	tbl->entry_array = (struct hash_table_entry*)malloc(sizeof(struct hash_table_entry) * tbl_sz);
#endif
	if(!tbl->entry_array)
		return ret;
	for(i = 0; i < tbl_sz; i++)
	{
		
#ifdef __KERNEL__
		tbl->entry_array[i].hash_id = (void*)kmalloc(tbl->hash_id_length, GFP_ATOMIC);
		tbl->entry_array[i].data = (void*)kmalloc(tbl->data_length, GFP_ATOMIC);
#else
		tbl->entry_array[i].hash_id = (void*)malloc(tbl->hash_id_length);
		tbl->entry_array[i].data = (void*)malloc(tbl->data_length);
#endif
		if(!tbl->entry_array[i].hash_id || !tbl->entry_array[i].data)
		{
			remove_hash_table(tbl);
			return ret;			
		}
		memset(&tbl->entry_array[i].dirty, 0, sizeof(unsigned char));
	}
	tbl->hash_function = hash_func;
	tbl->search_entry = search_func;
	tbl->insert_entry = insert_func;
	tbl->delete_entry = delete_func;
	tbl->traverse_table = traverse_func;
	ret = HASH_TABLE_SUCCEEDED;
	
	return ret;
}

long crc_hashing (struct hash_table* tbl, void* hid)
{
	unsigned char mask = 0;
	unsigned long i, index, tmp;
	
	if(tbl->table_size_power > 8)
	{
#ifdef __KERNEL__
		printk("The table size must not be greater than 256!!\n");
#else
		fprintf(stderr, "The table size must not be greater than 256!!\n");
#endif
		return -1;
	}

	mask |= 0x01;
	for(i = 1; i < tbl->table_size_power; i++)
	{
		mask <<= 1;
		mask |= 0x01;
	}

	for(i = 0, index = 0; i < tbl->hash_id_length; i++)
	{
		tmp = *(unsigned char*)(hid + i) & mask;
		index ^= tmp;
	}

	return index;
}

//by pepsi
long PU_hashing (struct hash_table* tbl, void* hid)
{
	unsigned char index;
	
	index = (*(unsigned char *)hid) & 0xff;

	return index;
}


void* search_default (struct hash_table* tbl, void* hid)
{
	unsigned long i, tbl_sz;
	long index;

	tbl_sz = 1 << tbl->table_size_power;
	index = tbl->hash_function(tbl, hid);

	if(index == -1)
		return NULL;

	for(i = index; i < tbl_sz; i++)
		if(tbl->entry_array[i].dirty != 0 && !memcmp(hid, tbl->entry_array[i].hash_id, tbl->hash_id_length))
			return tbl->entry_array[i].data;

	for(i = 0; i < index; i++)
		if(tbl->entry_array[i].dirty != 0 && !memcmp(hid, tbl->entry_array[i].hash_id, tbl->hash_id_length))
			return tbl->entry_array[i].data;

	return NULL;
}

long insert_default (struct hash_table* tbl, void* hid, void* data)
{
	// int prt=0;
	unsigned long i, available, tbl_sz;
	long index, ret = HASH_TABLE_FAILED;

	tbl_sz = 1 << tbl->table_size_power;
	index = tbl->hash_function(tbl, hid);

	if(index == -1)
		return ret;

	for(i = index, available = tbl_sz; i < tbl_sz; i++)
		if(available == tbl_sz && tbl->entry_array[i].dirty == 0)
				available = i;
		else if(tbl->entry_array[i].dirty != 0 && !memcmp(hid, tbl->entry_array[i].hash_id, tbl->hash_id_length))
			goto INSERT_LABEL2;

	for(i = 0; i < index; i++)
		if(available == tbl_sz && tbl->entry_array[i].dirty == 0)
				available = i;
		else if(tbl->entry_array[i].dirty != 0 && !memcmp(hid, tbl->entry_array[i].hash_id, tbl->hash_id_length))
			goto INSERT_LABEL2;

	if( available != tbl_sz)
	{
		i = available ;
		memcpy(tbl->entry_array[i].hash_id, hid, tbl->hash_id_length);
		tbl->entry_array[i].dirty = 0xff;
INSERT_LABEL2:					
		if(data != NULL)
			memcpy(tbl->entry_array[i].data, data, tbl->data_length);
		ret = HASH_TABLE_SUCCEEDED;
	}

	return ret;
}

long delete_default (struct hash_table* tbl, void* hid)
{
	unsigned long i,  tbl_sz;
	long index, ret = HASH_TABLE_FAILED;

	tbl_sz = 1 << tbl->table_size_power;
	index = tbl->hash_function(tbl, hid);

	if(index == -1)
		return ret;

	for(i = index ; i < tbl_sz; i++)
		if(tbl->entry_array[i].dirty != 0 && !memcmp(hid, tbl->entry_array[i].hash_id, tbl->hash_id_length))
			goto DELETE_LABEL;

	for(i = 0; i < index; i++)
		if(tbl->entry_array[i].dirty != 0 && !memcmp(hid, tbl->entry_array[i].hash_id, tbl->hash_id_length))
			break;			
		
	if(i != index)
	{
DELETE_LABEL:

		tbl->entry_array[i].dirty = 0;
		return HASH_TABLE_SUCCEEDED;
	}

	return ret;
}

void traverse_default (struct hash_table* tbl, void* handler)
{
	unsigned long i = 0, tbl_sz;
	void (*func)(void*);

	func = handler;
	tbl_sz = 1 << tbl->table_size_power;
	
	for(i = 0; i < tbl_sz; i++)
		if(tbl->entry_array[i].dirty != 0)
			func(tbl->entry_array[i].data);	
}

