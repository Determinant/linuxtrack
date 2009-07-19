#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "pref_int.h"

int yyparse(void);
extern FILE* yyin;
extern int yydebug;
extern char* yytext;

char *parsed_file;
int line_num;

plist prefs;

bool read_already = false;
char *pref_file = ".linuxtrack";

bool read_prefs(char *fname, char *section)
{
  prefs = create_list();
  if((yyin=fopen(fname, "r")) != NULL){
    parsed_file = my_strdup(fname);
    line_num = 1;
    //yydebug=1;
    int res = yyparse();
    free(parsed_file);
    parsed_file = NULL;
    if(res == 0){
      log_message("Preferences read OK!\n");
      return(true);
    }
  }
  log_message("Error encountered while reading preferences!\n");
  return(false);
}

void yyerror(char const *s)
{
  log_message("%s in file %s, line %d near '%s'\n",
  		 s, parsed_file, line_num, yytext);
}

char *get_pref_file_name()
{
  char *home = getenv("HOME");
  if(home == NULL){
    log_message("Please set HOME variable!'n");
    return NULL;
  }
  char *pref_path = (char *)my_malloc(strlen(home) 
                    + strlen(pref_file) + 2);
  sprintf(pref_path, "%s/%s", home, pref_file);
  return pref_path;
}

bool read_prefs_on_init()
{
  static bool read_already = false;
  static bool prefs_ok = false;
  if(read_already == false){
    char *pfile = get_pref_file_name();
    if(pfile == NULL){
      return false;
    }
    prefs_ok = read_prefs(pfile, "");
    free(pfile);
    read_already = true;
  }
  return prefs_ok;
}

section_struct *find_section(char *section_name)
{
  if(read_prefs_on_init() == false){
    return NULL;
  }
  iterator i;
  init_iterator(prefs, &i);
  
  pref_file_item *pfi;
  while((pfi = (pref_file_item *)get_next(&i)) != NULL){
    if(pfi->item_type == SECTION){
      assert(pfi->section != NULL);
      if(strcmp(pfi->section->name, section_name) == 0){
        return pfi->section;
      }
    }
  }
  return NULL;
}

key_val_struct *find_key(char *section_name, char *key_name)
{
  if(read_prefs_on_init() == false){
    return NULL;
  }
  section_struct *sec = find_section(section_name);
  if(sec == NULL){
    return NULL;
  }
  iterator i;
  init_iterator(sec->contents, &i);
  
  section_item *si;
  while((si = (section_item *)get_next(&i)) != NULL){
    if(si->sec_item_type == KEY_VAL){
      assert(si->key_val != NULL);
      if(strcmp(si->key_val->key, key_name) == 0){
        return si->key_val;
      }
    }
  }
  return NULL;
}

bool section_exists(char *section_name)
{
  if(read_prefs_on_init() == false){
    return false;
  }
  if(find_section(section_name) != NULL){
    return true;
  }else{
    return false;
  }
}

bool key_exists(char *section_name, char *key_name)
{
  if(read_prefs_on_init() == false){
    return false;
  }
  if(find_key(section_name, key_name) != NULL){
    return true;
  }else{
    return false;
  }
}

char *get_key(char *section_name, char *key_name)
{
  if(read_prefs_on_init() == false){
    return NULL;
  }
  key_val_struct *kv = find_key(section_name, key_name);
  if(kv == NULL){
    return NULL;
  }
  return kv->value;
}

/*
int main(int argc, char *argv[])
{
  printf("Device type: %s\n", get_key("Global", "Capture-device"));
  printf("Head ref [0, %s, %s]\n", get_key("Global", "Head-Y"), 
  	get_key("Global", "Head-Z"));
  return 0;
}
*/