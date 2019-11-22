#!/bin/sh

DIR="package/kernel/rtl8192cd"
U_FILE="package/kernel/rtl8192cd/unifdef-file"

_dos2unix(){
	find $DIR -type f -exec dos2unix '{}' ';'
	find $DIR -type f -name '*.[ch]' -exec sed -i 's/[ \t]*$//' '{}' ';'
}

_unifdef(){
	find $DIR -type f -name '*.[ch]' -exec unifdef -m -f $U_FILE '{}' ';'
}

_dos2unix
_unifdef
