/**
  * GreenPois0n Cynanide - aes.c
  * Copyright (C) 2010 Chronic-Dev Team
  * Copyright (C) 2010 Joshua Hill
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aes.h"
#include "nvram.h"
#include "common.h"
#include "commands.h"
#include "functions.h"

int(*aes_crypto_cmd)(AesOption option, void* input, void* output, unsigned int size, AesMode mode, void* iv, void* key) = NULL;

int aes_init() {
	//printf("Initializing aes\n");
	aes_crypto_cmd = find_function("aes_crypto_cmd", TARGET_BASEADDR, TARGET_BASEADDR);
	if(aes_crypto_cmd == NULL) {
		puts("Unable to find aes_crypto_cmd\n");
	} else {
		printf("Found aes_crypto_cmd at 0x%x\n", aes_crypto_cmd);
		cmd_add("aes", &aes_cmd, "encrypt/decrypt kbag aes keys using gid");
	}

	return 0;
}

int aes_cmd(int argc, CmdArg* argv) {
	int i = 0;
	char* kbag = NULL;
	char* action = NULL;
	unsigned int size = 0;
	unsigned char* rawkey = NULL;

	if(argc < 3 || argc > 4) {
		puts("usage: aes <enc/dec> [data]\n");
		return 0;
	}

	kbag = argv[2].string;
	action = argv[1].string;
	rawkey = (unsigned char*) malloc(kAesSizeMax);
	if(!strcmp(action, "dec")) {
		size = aes_decrypt_key(kbag, &rawkey);

	} else if(!strcmp(action, "enc")) {
		size = aes_encrypt_key(kbag, &rawkey);

	} else {
		free(rawkey);
		return -1;
	}
	
	char* iv = (char*) malloc(size * 2 + 1);
	char* key = (char*) malloc(size * 2 + 1);
	
	enter_critical_section();
	// set nvram vars for iv and key and print them
	for(i = 0; i < 16; i++) {
		sprintf(iv + i * 2, "%02x", rawkey[i]);
	}
	for(i = 0; i < size - 16; i++) {
		sprintf(key + i * 2, "%02x", rawkey[i+16]);
	}
	nvram_set_var("iv", iv);
	nvram_set_var("key", key);
	printf("IV: %s\n", iv);
	printf("Key: %s\n", key);
	exit_critical_section();
	
	if(rawkey) free(rawkey);
	if(iv) free(iv);
	if(key) free(key);
	return 0;
}

unsigned int aes_decrypt_key(unsigned char* in, unsigned char** out) {
	int i = 0;
	unsigned int size = 0;
	unsigned int byte = 0;
	unsigned char* data = *out;
	if(data == NULL) {
		return 0;
	}

	size = strlen(in) / 2;
	if(size > kAesSizeMax || size < kAesSizeMin) {
		return 0;
	}

	for(i = 0; i < size; i++) {
		sscanf(in, "%02x", &byte);
		data[i] = byte;
		in += 2;
	}

	aes_crypto_cmd(kAesDecrypt, data, data, size, kAesTypeGid, 0, 0);
	return size;
}

unsigned int aes_encrypt_key(unsigned char* in, unsigned char** out) {
	int i = 0;
	unsigned int size = 0;
	unsigned int byte = 0;
	unsigned char* data = *out;
	if(data == NULL) {
		return 0;
	}

	size = strlen(in) / 2;
	if(size > kAesSizeMax || size < kAesSizeMin) {
		return 0;
	}

	for(i = 0; i < size; i++) {
		sscanf(in, "%02x", &byte);
		data[i] = byte;
		in += 2;
	}

	aes_crypto_cmd(kAesEncrypt, data, data, size, kAesTypeGid, 0, 0);
	return size;
}
