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
 Suite* eas_email_info_translator_suite (void);
static  void test_eas_email_info_translator_add(const char* Serializeddata,const char* xmlName)
 {
	
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
	size_t readResult;
	long lSize;
	gchar *ptr= NULL;
	long size;
	gchar *buf;
  //endregion
#if !GLIB_CHECK_VERSION(2,36,0)
	g_type_init();
#endif
  // check the XML file, did the XML file exists
	size = pathconf(".", _PC_PATH_MAX);
	if ((buf = (char *)malloc((size_t)size)) != NULL)
	ptr = getcwd(buf, (size_t)size);
	fail_if(!stat(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Response/XML_Data/",xmlName,NULL),&stFileInfo)==0,"The test file from XML_Data folder does not exist,Please check your XML_Data folder.(check_tests/TestData/Email_Info_Translator/_Response/XML_Data/");	
  //end checking

  //Load XML Root Node
	doc = xmlParseFile(g_strconcat (ptr,  "/TestData/Email_Info_Translator/_Response/XML_Data/",xmlName,NULL));
  //end Loading

  //region Translate
	fail_if(doc==NULL,"The test file from XML_Data folder does not have good structure", "Please check your XML_Data folder.(check_tests/TestData/Email_Info_Translator/_Response/XML_Data/");
	nodeLevel1 = doc->children;
	result = eas_email_info_translator_parse_add_response(nodeLevel1, server_id);
	fw = fopen (g_strconcat (ptr,"test.txt",NULL),"w+");
	fputs (result,fw);
	fclose (fw);
  //end translation  //end Translation

  //load serialized test data
	fail_if(!stat(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Response/Serialized_Data/",Serializeddata,NULL),&stFileInfo)==0,"The test file from Serialized_Data folder does not exist,Please check your Serialized_Data folder.(check_tests/TestData/_Sync_Received_Email_Info/_Response/Serialized_Data/");	
	fr = fopen(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Response/Serialized_Data/",Serializeddata,NULL), "r");
	fail_if(fr==NULL,"The test file from Serialized_Data folder does not have good structure.", "Please check your Serialized_Data folder.(check_tests/TestData/Email_Info_Translator/_Response/Serialized_Data/");
	
	fseek (fr , 0 , SEEK_END);
	lSize = ftell (fr);
	rewind (fr);
	buffer = (gchar*) malloc (sizeof(gchar)*lSize+1);
	readResult=fread (buffer,sizeof(gchar),lSize,fr);
	fail_if(readResult == 0);
	buffer[lSize]= '\0';
  //end loading serialized data
	fail_if(g_strcmp0 (buffer,result)!=0, "The XML file it was not properly translated. Please check input data. In other case, function does not work properly.");
 }
START_TEST(test_eas_email_info_translator_add_all)
{
	test_eas_email_info_translator_add("eas_email_info_translator_add_all.txt","eas_email_info_translator_add_all.xml");
} 
END_TEST
START_TEST(test_eas_email_info_translator_add_attachment)
{
	test_eas_email_info_translator_add("eas_email_info_translator_add_attachment.txt","eas_email_info_translator_add_attachment.xml");
} 
END_TEST
START_TEST(test_eas_email_info_translator_add_body)
{
	test_eas_email_info_translator_add("eas_email_info_translator_add_body.txt","eas_email_info_translator_add_body.xml");
} 
END_TEST
START_TEST(test_eas_email_info_translator_add_category)
{
	test_eas_email_info_translator_add("eas_email_info_translator_add_category.txt","eas_email_info_translator_add_category.xml");
} 
END_TEST


//---------------------------------------------------Update------------------------------------------------------------


static void test_eas_email_info_translator_update(const char* Serializeddata,const char* xmlName)
 {   
  //server_id initialization
	gchar temp[]="3:1";
	gchar * server_id=g_strdup(temp);
  //end initialization of server_id
   
  //region init variable
	xmlDocPtr doc;
	xmlNodePtr nodeLevel1;
	struct stat stFileInfo;
	FILE *fr;
	gchar * result=NULL;
	gchar * buffer=NULL;
	 size_t readResult;
	long lSize;
	gchar *ptr= NULL;
	long size;
	gchar *buf;
g_type_init();
  //endregion

  // check the XML file, did the XML file exists
	
	size = pathconf(".", _PC_PATH_MAX);
	if ((buf = (char *)malloc((size_t)size)) != NULL)
	ptr = getcwd(buf, (size_t)size);
	fail_if(!stat(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Response/XML_Data/",xmlName,NULL),&stFileInfo)==0,"The test file from XML_Data folder does not exist,Please check your XML_Data folder.(check_tests/TestData/Email_Info_Translator/_Response/XML_Data/");	
  //end checking

  //Load XML Root Node
	doc = xmlParseFile(g_strconcat (ptr,  "/TestData/Email_Info_Translator/_Response/XML_Data/",xmlName,NULL));
  //end Loading

  //region Translate
	fail_if(doc==NULL,"The test file from XML_Data folder does not have good structure", "Please check your XML_Data folder.(check_tests/TestData/Email_Info_Translator/_Response/XML_Data/");
	nodeLevel1 = doc->children;
	result = eas_email_info_translator_parse_update_response(nodeLevel1, server_id);

  //end translation

  //load serialized test data
  
	fail_if(!stat(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Response/Serialized_Data/",Serializeddata,NULL),&stFileInfo)==0,"The test file from Serialized_Data folder does not exist,Please check your Serialized_Data folder.(check_tests/TestData/Email_Info_Translator/_Response/Serialized_Data/");	
	fr = fopen(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Response/Serialized_Data/",Serializeddata,NULL), "r");
	fail_if(fr==NULL,"The test file from Serialized_Data folder does not have good structure.", "Please check your Serialized_Data folder.(check_tests/TestData/Email_Info_Translator/_Response/Serialized_Data/");
	
	fseek (fr , 0 , SEEK_END);
	lSize = ftell (fr);
	rewind (fr);
	buffer = (gchar*) malloc (sizeof(gchar)*lSize+1);
	readResult=fread (buffer,sizeof(gchar),lSize,fr);
	fail_if(readResult == 0);
	buffer[lSize]= '\0';
  //end loading serialized data
	fail_if(g_strcmp0 (buffer,result)!=0, "The XML file it was not properly translated. Please check input data. In other case, function does not work properly.");
	
 }

START_TEST(test_eas_email_info_translator_update_all)
{
	test_eas_email_info_translator_update("eas_email_info_translator_update_all.txt","eas_email_info_translator_update_all.xml");
} 
END_TEST
START_TEST(test_eas_email_info_translator_update_category)
{
	test_eas_email_info_translator_update("eas_email_info_translator_update_category.txt","eas_email_info_translator_update_category.xml");
} 
END_TEST
START_TEST(test_eas_email_info_translator_update_read)
{
	test_eas_email_info_translator_update("eas_email_info_translator_update_read.txt","eas_email_info_translator_update_read.xml");
} 
END_TEST

static void test_eas_email_info_translator_delete(const char* Serializeddata,const char* xmlName)
 {
	
  //server_id initialization
	gchar temp[]="3:1";
	gchar * server_id=g_strdup(temp);
  //end initialization of server_id
   
  //region init variable
	xmlDocPtr doc;
	xmlNodePtr nodeLevel1;
	struct stat stFileInfo;
	FILE *fr;
	gchar * result=NULL;
	gchar * buffer=NULL;
	 size_t readResult;
	long lSize;
	gchar *ptr= NULL;
	long size;
	gchar *buf;

	g_type_init();   
  //endregion

  // check the XML file, did the XML file exists	
	size = pathconf(".", _PC_PATH_MAX);
	if ((buf = (char *)malloc((size_t)size)) != NULL)
	ptr = getcwd(buf, (size_t)size);
	fail_if(!stat(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Response/XML_Data/",xmlName,NULL),&stFileInfo)==0,"The test file from XML_Data folder does not exist,Please check your XML_Data folder.(check_tests/TestData/Email_Info_Translator/_Response/XML_Data/");	
  //end checking

  //Load XML Root Node
	doc = xmlParseFile(g_strconcat (ptr,  "/TestData/Email_Info_Translator/_Response/XML_Data/",xmlName,NULL));
  //end Loading

  //region Translate
	fail_if(doc==NULL,"The test file from XML_Data folder does not have good structure", "Please check your XML_Data folder.(check_tests/TestData/Email_Info_Translator/_Response/XML_Data/");
	nodeLevel1 = doc->children;
	result = eas_email_info_translator_parse_delete_response(nodeLevel1, server_id);

  //end translation 

  //load serialized test data
  
	fail_if(!stat(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Response/Serialized_Data/",Serializeddata,NULL),&stFileInfo)==0,"The test file from Serialized_Data folder does not exist,Please check your Serialized_Data folder.(check_tests/TestData/Email_Info_Translator/_Response/Serialized_Data/");	
	fr = fopen(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Response/Serialized_Data/",Serializeddata,NULL), "r");
	fail_if(fr==NULL,"The test file from Serialized_Data folder does not have good structure.", "Please check your Serialized_Data folder.(check_tests/TestData/Email_Info_Translator/_Response/Serialized_Data/");
	
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
	test_eas_email_info_translator_update("eas_email_info_translator_delete_all.txt","eas_email_info_translator_delete_all.xml");
} 
END_TEST
//---------------------------------------------------Request------------------------------------------------------------


static void test_eas_mail_info_translator_build_update_request(const char* serializedData,const char* xmlName)
{
  //region init variable
	xmlDocPtr doc;
	xmlNodePtr nodeLevel1;
	FILE *fr,*fw;
	long lSize1,lSize2,lSize3;
	gchar * buffer1=NULL;
	gchar * buffer2=NULL;
	gchar* 	buffer3=NULL;
	struct stat stFileInfo;
	EasEmailInfo *email_info;
	size_t readResult;
	gchar *ptr= NULL;
	long size;
	gchar *buf;
	gboolean parseResponse;

	g_type_init();
	 email_info = eas_email_info_new();
//endregion
//check the SerializedData file, did the serializedData file exists
	size = pathconf(".", _PC_PATH_MAX);
	if ((buf = (char *)malloc((size_t)size)) != NULL)
	ptr = getcwd(buf, (size_t)size);
	fail_if(!stat(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Request/Serialized_Data/",serializedData,NULL),&stFileInfo)==0,"The test file from Serialized_Data folder does not exist,Please check your Serialized_Data folder.(/TestData/Email_Info_Translator/_Request/Serialized_Data/)");
//end checking

//region Load Serialized data
	fr = fopen(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Request/Serialized_Data/",serializedData,NULL), "r");
	fail_if(fr==NULL,"The test file from Serialized_Data folder does not have good structure.", "Please check your Serialized_Data folder.(/TestData/Email_Info_Translator/_Request/Serialized_Data/)");
	fseek (fr , 0 , SEEK_END);
	lSize1 = ftell (fr);
	rewind (fr);
	buffer1 = (gchar*) malloc (sizeof(gchar)*lSize1 + 1);
	readResult=fread (buffer1,sizeof(gchar),lSize1,fr);
	fail_if(readResult == 0);
	buffer1[lSize1]='\0';
	eas_email_info_deserialise(email_info,buffer1);
	fclose (fr);
//end Loading

//Load XML Root Node
	doc = xmlParseFile(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Request/XML_Data/temp.xml",NULL));
//end Loading

//region Translate
	fail_if(doc==NULL,"The test file from XML_Data folder does not have good structure", "Please check your XML_Data folder.(check_tests/TestData/Email_Info_Translator/_Request/Serialized_Data/");
	nodeLevel1 = doc->children;
	parseResponse = eas_email_info_translator_build_update_request(doc, nodeLevel1, email_info);
	fail_unless(parseResponse, "XML can't be created.");
//end Translation

//region Save Translation in temp.txt	
		fw = fopen(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Request/XML_Data/temp.txt",NULL), "w");
		xmlDocFormatDump(fw,doc,1);
	fclose(fw);
	xmlFreeDoc(doc);
//end region

//region Load Translation
fr = fopen(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Request/XML_Data/temp.txt",NULL), "r");
	fail_if(fr==NULL,"The temp file from XML_Data folder does not have good structure.", "Please check your XML_Data folder.(/TestData/Email_Info_Translator/_Request/XML_Data/)");
	fseek (fr , 0 , SEEK_END);
	lSize2 = ftell (fr);
	rewind (fr);
	buffer2 = (gchar*) malloc (sizeof(gchar)*lSize2 + 1);
	readResult=fread (buffer2,sizeof(gchar),lSize2,fr);
	fail_if(readResult == 0);
	buffer2[lSize2]='\0';
	fclose (fr);
	remove(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Request/XML_Data/temp.txt",NULL)); //Delete the temporary File temp.txt
//end Loading Translation

//region Load XML Data 
fr = fopen(g_strconcat (ptr, "/TestData/Email_Info_Translator/_Request/XML_Data/",xmlName,NULL), "r");
	fail_if(fr==NULL,"The test file from XML_Data folder does not have good structure.", "Please check your XML_Data folder.(/TestData/Email_Info_Translator/_Request/Serialized_Data/)");
	fseek (fr , 0 , SEEK_END);
	lSize3 = ftell (fr);
	rewind (fr);
	buffer3 = (gchar*) malloc (sizeof(gchar)*lSize3 + 1);
	readResult=fread (buffer3,sizeof(gchar),lSize3,fr);
	fail_if(readResult == 0);
	buffer3[lSize3]='\0';
	
	fclose (fr);

	fail_if(g_strcmp0 (buffer2,buffer3)!=0, "The EmailInfo file it was not properly translated. Please check input data. In other case, function does not work properly.");

}
START_TEST(test_eas_email_info_translator_build_update_request_all)
{
test_eas_mail_info_translator_build_update_request("eas_email_info_translator_build_update_request_all.txt","eas_email_info_translator_build_update_request_all.xml");
} 
END_TEST
START_TEST(test_eas_email_info_translator_build_update_request_category)
{
test_eas_mail_info_translator_build_update_request("eas_email_info_translator_build_update_request_category.txt","eas_email_info_translator_build_update_request_category.xml");
} 
END_TEST
START_TEST(test_eas_email_info_translator_build_update_request_read)
{
test_eas_mail_info_translator_build_update_request("eas_email_info_translator_build_update_request_read.txt","eas_email_info_translator_build_update_request_read.xml");
} 
END_TEST
Suite* eas_email_info_translator_suite (void)
{
    Suite* s = suite_create ("eas_email_info_translator");

    /* con-info-translator test case */
    TCase *tc_email_info_translator = tcase_create ("core");
    suite_add_tcase (s, tc_email_info_translator);

	tcase_add_test (tc_email_info_translator,test_eas_email_info_translator_add_all);
	tcase_add_test (tc_email_info_translator,test_eas_email_info_translator_add_attachment);
	tcase_add_test (tc_email_info_translator,test_eas_email_info_translator_add_body);
	tcase_add_test (tc_email_info_translator,test_eas_email_info_translator_add_category);

	tcase_add_test (tc_email_info_translator,test_eas_email_info_translator_update_all);
	tcase_add_test (tc_email_info_translator,test_eas_email_info_translator_update_category);
	tcase_add_test (tc_email_info_translator,test_eas_email_info_translator_update_read);

	tcase_add_test (tc_email_info_translator, test_eas_email_info_translator_delete_all);

   	tcase_add_test (tc_email_info_translator,test_eas_email_info_translator_build_update_request_read);
	tcase_add_test (tc_email_info_translator,test_eas_email_info_translator_build_update_request_all);
	tcase_add_test (tc_email_info_translator,test_eas_email_info_translator_build_update_request_category);

	return s;
}
