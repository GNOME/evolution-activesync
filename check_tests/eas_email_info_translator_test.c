#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>
#include <dbus/dbus-glib.h>
#include <sys/stat.h>

#include "eas_test_user.h"
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <unistd.h>

#include <time.h>
#include "../eas-daemon/libeas/eas-email-info-translator.h"
#include "../libeasmail/src/eas-folder.h"
#include "../libeasmail/src/eas-attachment.h"
#include "../libeasmail/src/eas-email-info.h"
#include <libxml/tree.h>
#include <string.h>
#include <ctype.h>
/** Uncomment for to enable Mocks **/
 #include "../libeastest/src/libeastest.h"
 
 void test_eas_email_info_translator_add(const char* Serializeddata,const char* xmlName)
 {
	g_type_init();
   
  //server_id initialization
	gchar temp[]="3:1";
	gchar * server_id=g_strdup(temp);
  //end initialization of server_id
   
  //region init variable
	xmlDocPtr doc;
	xmlNodePtr nodeLevel1;
	struct stat stFileInfo;
	FILE *fr,*fw;
	gchar * result=NULL;
	gchar * buffer=NULL;
	gchar * readResult=NULL;
	long lSize;
  //endregion

  // check the XML file, did the XML file exists
	gchar *ptr= NULL;
	long size;
	gchar *buf;
	size = pathconf(".", _PC_PATH_MAX);
	if ((buf = (char *)malloc((size_t)size)) != NULL)
	ptr = getcwd(buf, (size_t)size);
	fail_if(!stat(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Sync_Received_Email_Info/XML_Data/",xmlName,NULL),&stFileInfo)==0,"The test file from XML_Data folder does not exist,Please check your XML_Data folder.(check_tests/TestData/Email_Info_Translator/_Sync_Received_Email_Info/XML_Data/");	
  //end checking

  //Load XML Root Node
	doc = xmlParseFile(g_strconcat (ptr,  "/TestData/Email_Info_Translator/_Sync_Received_Email_Info/XML_Data/",xmlName,NULL));
  //end Loading

  //region Translate
	fail_if(doc==NULL,"The test file from XML_Data folder does not have good structure", "Please check your XML_Data folder.(check_tests/TestData/Email_Info_Translator/_Sync_Received_Email_Info/XML_Data/");
	nodeLevel1 = doc->children;
	result = eas_email_info_translator_parse_add_response(nodeLevel1, server_id);
  //end translation  //end Translation

  //load serialized test data
	fail_if(!stat(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Sync_Received_Email_Info/Serialized_Data/",Serializeddata,NULL),&stFileInfo)==0,"The test file from Serialized_Data folder does not exist,Please check your Serialized_Data folder.(check_tests/TestData/Email_Info_Translator/_Sync_Received_Email_Info/Serialized_Data/");	
	fr = fopen(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Sync_Received_Email_Info/Serialized_Data/",Serializeddata,NULL), "r");
	fail_if(fr==NULL,"The test file from Serialized_Data folder does not have good structure.", "Please check your Serialized_Data folder.(check_tests/TestData/Email_Info_Translator/_Sync_Received_Email_Info/Serialized_Data/");
	
	fseek (fr , 0 , SEEK_END);
	lSize = ftell (fr);
	rewind (fr);
	buffer = (gchar*) malloc (sizeof(gchar)*lSize+1);
	readResult=fread (buffer,sizeof(gchar),lSize,fr);
	fail_if(readResult == 0);
	buffer[lSize]='\0';
  //end loading serialized data
	fail_if(g_strcmp0 (buffer,result)!=0, "The XML file it was not properly translated. Please check input data. In other case, function does not work properly.");
 }
 
START_TEST(test_eas_email_info_translator_add_all)
{
	test_eas_email_info_translator_add("output.txt","input.xml");
} 
END_TEST

// TODO -> We should create cases, in which we will test separatly fields.


//---------------------------------------------------Update------------------------------------------------------------

void test_eas_email_info_translator_update(const char* Serializeddata,const char* xmlName)
 {
	g_type_init();
   
  //server_id initialization
	gchar temp[]="3:1";
	gchar * server_id=g_strdup(temp);
  //end initialization of server_id
   
  //region init variable
	xmlDocPtr doc;
	xmlNodePtr nodeLevel1;
	struct stat stFileInfo;
	FILE *fr,*fw;
	gchar * result=NULL;
	gchar * buffer=NULL;
	gchar * readResult=NULL;
	long lSize;
  //endregion

  // check the XML file, did the XML file exists
	gchar *ptr= NULL;
	long size;
	gchar *buf;
	size = pathconf(".", _PC_PATH_MAX);
	if ((buf = (char *)malloc((size_t)size)) != NULL)
	ptr = getcwd(buf, (size_t)size);
	fail_if(!stat(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Sync_Received_Email_Info/XML_Data/",xmlName,NULL),&stFileInfo)==0,"The test file from XML_Data folder does not exist,Please check your XML_Data folder.(check_tests/TestData/Email_Info_Translator/_Sync_Received_Email_Info/XML_Data/");	
  //end checking

  //Load XML Root Node
	doc = xmlParseFile(g_strconcat (ptr,  "/TestData/Email_Info_Translator/_Sync_Received_Email_Info/XML_Data/",xmlName,NULL));
  //end Loading

  //region Translate
	fail_if(doc==NULL,"The test file from XML_Data folder does not have good structure", "Please check your XML_Data folder.(check_tests/TestData/Email_Info_Translator/_Sync_Received_Email_Info/XML_Data/");
	nodeLevel1 = doc->children;
	result = eas_email_info_translator_parse_update_response(nodeLevel1, server_id);
  //end translation  //end Translation

  //load serialized test data
  
	fail_if(!stat(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Sync_Received_Email_Info/Serialized_Data/",Serializeddata,NULL),&stFileInfo)==0,"The test file from Serialized_Data folder does not exist,Please check your Serialized_Data folder.(check_tests/TestData/Email_Info_Translator/_Sync_Received_Email_Info/Serialized_Data/");	
	fr = fopen(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Sync_Received_Email_Info/Serialized_Data/",Serializeddata,NULL), "r");
	fail_if(fr==NULL,"The test file from Serialized_Data folder does not have good structure.", "Please check your Serialized_Data folder.(check_tests/TestData/Email_Info_Translator/_Sync_Received_Email_Info/Serialized_Data/");
	
	fseek (fr , 0 , SEEK_END);
	lSize = ftell (fr);
	rewind (fr);
	buffer = (gchar*) malloc (sizeof(gchar)*lSize+1);
	readResult=fread (buffer,sizeof(gchar),lSize,fr);
	fail_if(readResult == 0);
	buffer[lSize]='\0';
  //end loading serialized data
	g_debug("my result =%s", result);
	fail_if(g_strcmp0 (buffer,result)!=0, "The XML file it was not properly translated. Please check input data. In other case, function does not work properly.");
	
 }

START_TEST(test_eas_email_info_translator_update_all)
{
	test_eas_email_info_translator_update("output2.txt","input2.xml");
} 
END_TEST

// TODO -> We should create cases, in which we will test separatly fields.

//--------------------------------------------------- Delete ------------------------------------------------------------

void test_eas_email_info_translator_delete(const char* Serializeddata,const char* xmlName)
 {
	g_type_init();
   
  //server_id initialization
	gchar temp[]="3:1";
	gchar * server_id=g_strdup(temp);
  //end initialization of server_id
   
  //region init variable
	xmlDocPtr doc;
	xmlNodePtr nodeLevel1;
	struct stat stFileInfo;
	FILE *fr,*fw;
	gchar * result=NULL;
	gchar * buffer=NULL;
	gchar * readResult=NULL;
	long lSize;
  //endregion

  // check the XML file, did the XML file exists
	gchar *ptr= NULL;
	long size;
	gchar *buf;
	size = pathconf(".", _PC_PATH_MAX);
	if ((buf = (char *)malloc((size_t)size)) != NULL)
	ptr = getcwd(buf, (size_t)size);
	fail_if(!stat(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Sync_Received_Email_Info/XML_Data/",xmlName,NULL),&stFileInfo)==0,"The test file from XML_Data folder does not exist,Please check your XML_Data folder.(check_tests/TestData/Email_Info_Translator/_Sync_Received_Email_Info/XML_Data/");	
  //end checking

  //Load XML Root Node
	doc = xmlParseFile(g_strconcat (ptr,  "/TestData/Email_Info_Translator/_Sync_Received_Email_Info/XML_Data/",xmlName,NULL));
  //end Loading

  //region Translate
	fail_if(doc==NULL,"The test file from XML_Data folder does not have good structure", "Please check your XML_Data folder.(check_tests/TestData/Email_Info_Translator/_Sync_Received_Email_Info/XML_Data/");
	nodeLevel1 = doc->children;
	result = eas_email_info_translator_parse_delete_response(nodeLevel1, server_id);
  //end translation  //end Translation

  //load serialized test data
  
	fail_if(!stat(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Sync_Received_Email_Info/Serialized_Data/",Serializeddata,NULL),&stFileInfo)==0,"The test file from Serialized_Data folder does not exist,Please check your Serialized_Data folder.(check_tests/TestData/Email_Info_Translator/_Sync_Received_Email_Info/Serialized_Data/");	
	fr = fopen(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Sync_Received_Email_Info/Serialized_Data/",Serializeddata,NULL), "r");
	fail_if(fr==NULL,"The test file from Serialized_Data folder does not have good structure.", "Please check your Serialized_Data folder.(check_tests/TestData/Email_Info_Translator/_Sync_Received_Email_Info/Serialized_Data/");
	
	fseek (fr , 0 , SEEK_END);
	lSize = ftell (fr);
	rewind (fr);
	buffer = (gchar*) malloc (sizeof(gchar)*lSize+1);
	readResult=fread (buffer,sizeof(gchar),lSize,fr);
	fail_if(readResult == 0);
	buffer[lSize]='\0';
  //end loading serialized data
	fail_if(g_strcmp0 (buffer,result)!=0, "The XML file it was not properly translated. Please check input data. In other case, function does not work properly.");
	
 }

START_TEST(test_eas_email_info_translator_delete_all)
{
	test_eas_email_info_translator_update("output2.txt","input2.xml");
} 
END_TEST

// TODO -> We need to create test for eas_email_info_translator_build_update_request function and for eas_email_info_translator_delete function.


Suite* eas_email_info_translator_suite (void)
{
    Suite* s = suite_create ("eas_email_info_translator");

    /* con-info-translator test case */
    TCase *tc_email_info_translator = tcase_create ("core");
    suite_add_tcase (s, tc_email_info_translator);

	//tcase_add_test (tc_email_info_translator,test_eas_email_info_translator_add_all);
	tcase_add_test (tc_email_info_translator,test_eas_email_info_translator_update_all);
    return s;
}
