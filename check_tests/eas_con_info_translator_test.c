#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>
#include <glib.h>
#include "eas_test_user.h"
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <sys/stat.h>
#include "../libeassync/src/libeassync.h"
#include "../libeassync/src/eas-item-info.h"
#include "../eas-daemon/libeas/eas-con-info-translator.h"
//#include <libebook/e-vcard.h>
#include <unistd.h> 

//-------------------------------------------------------------//
//   Input Vcard file it shoulde use UTF-8 with CR+LF format   //
//   To do that, you can use on linux  Leafpad 0.8.17          //
//-------------------------------------------------------------//
Suite* eas_con_info_translator_suite (void);
static void test_info_translator_parse_request(const char* vCardName,const char* xmlName)
{
      
  //region init variable
	xmlDocPtr doc;
	xmlNodePtr nodeLevel1;
	FILE *fr,*fw;
	gchar temp[]="3:1";
	gchar* serv=temp;
	long lSize1,lSize2,lSize3;
	gchar * buffer1=NULL;
	gchar * buffer2=NULL;
	gchar* 	buffer3=NULL;
	gchar *ptr= NULL;
	long size;
	gchar *buf;
	struct stat stFileInfo;
	EasItemInfo* conInfo = NULL;
	size_t readResult;

#if !GLIB_CHECK_VERSION(2,36,0)
 	g_type_init();
#endif
	conInfo = eas_item_info_new();
	conInfo->server_id = serv;
//endregion
//check the VCard file, did the VCard file exists
	size = pathconf(".", _PC_PATH_MAX);
	if ((buf = (char *)malloc((size_t)size)) != NULL)
	ptr = getcwd(buf, (size_t)size);
	 
	fail_if(!stat(g_strconcat (ptr, "/TestData/Con_Info_Translator/_Request/VCard_Data/",vCardName,NULL),&stFileInfo)==0,"The test file from VCard_Data folder does not exist,Please check your VCard_Data folder.(check_tests/TestData/Con_Info_Translator/_Request/VCard_Data");
//end checking

//region Load VCard data
	fr = fopen(g_strconcat (ptr, "/TestData/Con_Info_Translator/_Request/VCard_Data/",vCardName,NULL), "r");
	fail_if(fr==NULL,"The test file from VCard_Data folder does not have good structure.", "Please check your VCard_Data folder.(check_tests/TestData/Con_Info_Translator/_Request/VCard_Data)");
	fseek (fr , 0 , SEEK_END);
	lSize1 = ftell (fr);
	rewind (fr);
	buffer1 = (gchar*) malloc (sizeof(gchar)*lSize1 + 1);
	readResult = fread (buffer1,sizeof(gchar),lSize1,fr);
	fail_if(readResult==0);
	buffer1[lSize1]='\0';
	conInfo->data = buffer1;
	fclose (fr);
//end Loading

//Load XML Root Node
	doc = xmlParseFile(g_strconcat (ptr, "/TestData/Con_Info_Translator/_Request/XML_Data/temp.xml",NULL));
//end Loading

//region Translate
	fail_if(doc==NULL,"The test file from XML_Data folder does not have good structure", "Please check your XML_Data folder.(check_tests/TestData/Con_Info_Translator/XML_Data/");
	nodeLevel1 = doc->children;

	fail_if(eas_con_info_translator_parse_request(doc, nodeLevel1, conInfo)==FALSE, "XML can't be created.");
//end Translation

//region Save Translation in temp.txt	
		fw = fopen(g_strconcat (ptr, "/TestData/Con_Info_Translator/_Request/XML_Data/temp.txt",NULL), "w");
		xmlDocFormatDump(fw,doc,1);
	fclose(fw);
	xmlFreeDoc(doc);
//end region

//region Load Translation
fr = fopen(g_strconcat (ptr, "/TestData/Con_Info_Translator/_Request/XML_Data/temp.txt",NULL), "r");
	fail_if(fr==NULL,"The temp file from XML_Data folder does not have good structure.", "Please check your XML_Data folder.(check_tests/TestData/Con_Info_Translator/_Request/XML_Data)");
	fseek (fr , 0 , SEEK_END);
	lSize2 = ftell (fr);
	rewind (fr);
	buffer2 = (gchar*) malloc (sizeof(gchar)*lSize2 + 1);
	readResult = fread (buffer2,sizeof(gchar),lSize2,fr);
	fail_if(readResult==0);
	buffer2[lSize2]='\0';
	fclose (fr);
	remove(g_strconcat (ptr, "/TestData/Con_Info_Translator/_Request/XML_Data/temp.txt",NULL)); //Delete the temporary File temp.txt
//end Loading Translation

//region Load XML Data 
fr = fopen(g_strconcat (ptr, "/TestData/Con_Info_Translator/_Request/XML_Data/",xmlName,NULL), "r");
	fail_if(fr==NULL,"The test file from XML_Data folder does not have good structure.", "Please check your XML_Data folder.(check_tests/TestData/Con_Info_Translator/_Request/XML_Data)");
	fseek (fr , 0 , SEEK_END);
	lSize3 = ftell (fr);
	rewind (fr);
	buffer3 = (gchar*) malloc (sizeof(gchar)*lSize3 + 1);
	readResult = fread (buffer3,sizeof(gchar),lSize3,fr);
	fail_if(readResult==0);
	buffer3[lSize3]='\0';
	
	fclose (fr);
//end Loading XML Data


	fail_if(g_strcmp0 (buffer2,buffer3)!=0, "The VCard file it was not properly translated. Please check input data. In other case, function does not work properly.");

}


START_TEST (test_info_translator_parse_request_jobTitle)
{ 
test_info_translator_parse_request("VCard_info_translator_parse_request_jobTitle.txt","_XML_info_translator_parse_request_jobTitle.xml");
}
END_TEST

START_TEST (test_info_translator_parse_request_name)
{ 
test_info_translator_parse_request("VCard_info_translator_parse_request_name.txt","_XML_info_translator_parse_request_name.xml");
}
END_TEST

START_TEST (test_info_translator_parse_request_email_webPage)
{ 
test_info_translator_parse_request("VCard_info_translator_parse_request_email_webPage.txt","_XML_info_translator_parse_request_email_webPage.xml");
}
END_TEST

START_TEST (test_info_translator_parse_request_businessAdr)
{ 
test_info_translator_parse_request("VCard_info_translator_parse_request_businessAdr.txt","_XML_info_translator_parse_request_businessAdr.xml");
}
END_TEST

START_TEST (test_info_translator_parse_request_pagerNumber)
{ 
test_info_translator_parse_request("VCard_info_translator_parse_request_pagerNumber.txt","_XML_info_translator_parse_request_pagerNumber.xml");
}
END_TEST

START_TEST (test_info_translator_parse_request_birthday)
{ 
test_info_translator_parse_request("VCard_info_translator_parse_request_birthday.txt","_XML_info_translator_parse_request_birthday.xml");
}
END_TEST

START_TEST (test_info_translator_parse_request_blank)
{ 
test_info_translator_parse_request("VCard_info_translator_parse_request_blank.txt","_XML_info_translator_parse_request_blank.xml");
}
END_TEST

START_TEST (test_info_translator_parse_request_mobile)
{ 
test_info_translator_parse_request("VCard_info_translator_parse_request_mobile.txt","_XML_info_translator_parse_request_mobile.xml");
}
END_TEST

START_TEST (test_info_translator_parse_request_note)
{ 
test_info_translator_parse_request("VCard_info_translator_parse_request_note.txt","_XML_info_translator_parse_request_note.xml");
}
END_TEST


static void test_info_translator_parse_response(const char* vcardName, const char* xmlName)
{
//region init variable
	xmlDocPtr doc;
	xmlNodePtr nodeLevel1;
	FILE *fr;
	gchar temp[]="3:1";		       // !important = server_id array should only contains 3 characters 
	gchar* serv=g_strdup(temp);
	gchar* res = NULL;
	long lSize;
	gchar * buffer=NULL;
	struct stat stFileInfo;
	EasItemInfo *item;
	size_t readResult;
//endregion
//check the VCard file, did the VCard file exists

	gchar *ptr= NULL;
	long size;
	gchar *buf;
	size = pathconf(".", _PC_PATH_MAX);
	if ((buf = (char *)malloc((size_t)size)) != NULL)
	ptr = getcwd(buf, (size_t)size);

	fail_if(!stat(g_strconcat (ptr, "/TestData/Con_Info_Translator/_Response/VCard_Data/",vcardName,NULL),&stFileInfo)==0,"The test file from VCard_Data folder does not exist,Please check your VCard_Data folder.(check_tests/TestData/Con_Info_Translator/VCard_Data");
//end checking

//region Load VCard data
	fr = fopen(g_strconcat (ptr, "/TestData/Con_Info_Translator/_Response/VCard_Data/",vcardName,NULL), "r");
	fail_if(fr==NULL,"The test file from VCard_Data folder does not have good structure.", "Please check your VCard_Data folder.(check_tests/TestData/Con_Info_Translator/_Response/VCard_Data)");
	
	fseek (fr , 0 , SEEK_END);
	lSize = ftell (fr);
	rewind (fr);
	buffer = (gchar*) malloc (sizeof(gchar)*lSize+1);
	readResult=fread (buffer,sizeof(gchar),lSize,fr);
	fail_if(readResult == 0);
	buffer[lSize]='\0';
//endregion

//check the xml file, did the xml file exists
	fail_if(!stat(g_strconcat (ptr, "/TestData/Con_Info_Translator/_Response/XML_Data/",xmlName,NULL),&stFileInfo)==0,"The test file from XML_Data folder does not exist,Please check your XML_Data folder.(check_tests/TestData/Con_Info_Translator/_Response/XML_Data");
//end checking

//region Load XML test data
	doc = xmlParseFile(g_strconcat (ptr, "/TestData/Con_Info_Translator/_Response/XML_Data/",xmlName,NULL));
//endregion

	fail_if(doc==NULL,"The test file from XML_Data folder does not have good structure", "Please check your XML_Data folder.(check_tests/TestData/Con_Info_Translator/_Response/XML_Data");

	nodeLevel1 = doc->children;
	g_type_init();
	fail_if(nodeLevel1==NULL,"The XML file does not have correct structure");

	res = eas_con_info_translator_parse_response(nodeLevel1, serv);
	fclose (fr);

	item = eas_item_info_new();
	eas_item_info_deserialise(item,res);

	fail_if(g_strcmp0 (buffer,item->data)!=0, "The XML file it was not properly translated. Please check input data. In other case, function does not work properly.");
	
	xmlFreeDoc(doc);

}
START_TEST (test_info_translator_parse_response_name)
{
 
test_info_translator_parse_response("VCard_info_translator_parse_response_name.txt","_XML_info_translator_parse_response_name.xml");
}
END_TEST

START_TEST (test_info_translator_parse_response_email_webPage)
{ 
test_info_translator_parse_response("VCard_info_translator_parse_response_email_webPage.txt","_XML_info_translator_parse_response_email_webPage.xml");
}
END_TEST

START_TEST (test_info_translator_parse_response_jobTitle)
{ 
test_info_translator_parse_response("VCard_info_translator_parse_response_jobTitle.txt","_XML_info_translator_parse_response_jobTitle.xml");
}
END_TEST

START_TEST (test_info_translator_parse_response_all)
{ 
test_info_translator_parse_response("VCard_info_translator_parse_response_all.txt","_XML_info_translator_parse_response_all.xml");
}
END_TEST
START_TEST (test_info_translator_parse_response_businessAdr)
{ 
test_info_translator_parse_response("VCard_info_translator_parse_response_businessAdr.txt","_XML_info_translator_parse_response_businessAdr.xml");
}
END_TEST
START_TEST (test_info_translator_parse_response_otherAdr)
{ 
test_info_translator_parse_response("VCard_info_translator_parse_response_otherAdr.txt","_XML_info_translator_parse_response_otherAdr.xml");
}
END_TEST
START_TEST (test_info_translator_parse_response_pagerNumber)
{ 
test_info_translator_parse_response("VCard_info_translator_parse_response_pagerNumber.txt","_XML_info_translator_parse_response_pagerNumber.xml");
}
END_TEST

START_TEST (test_info_translator_parse_response_birthday)
{ 
test_info_translator_parse_response("VCard_info_translator_parse_response_birthday.txt","_XML_info_translator_parse_response_birthday.xml");
}
END_TEST
START_TEST (test_info_translator_parse_response_blank)
{ 
test_info_translator_parse_response("VCard_info_translator_parse_response_blank.txt","_XML_info_translator_parse_response_blank.xml");
}
END_TEST
START_TEST (test_info_translator_parse_response_mobile)
{ 
test_info_translator_parse_response("VCard_info_translator_parse_response_mobile.txt","_XML_info_translator_parse_response_mobile.xml");
}
END_TEST

START_TEST (test_info_translator_parse_response_note)
{ 
test_info_translator_parse_response("VCard_info_translator_parse_response_note.txt","_XML_info_translator_parse_response_note.xml");
}
END_TEST
 
Suite* eas_con_info_translator_suite (void)
{
    Suite* s = suite_create ("con_info_translator");

    /* con-info-translator test case */
    TCase *tc_con_info_translator = tcase_create ("core");
    suite_add_tcase (s, tc_con_info_translator);

	tcase_add_test (tc_con_info_translator,test_info_translator_parse_request_jobTitle);
	tcase_add_test(tc_con_info_translator,test_info_translator_parse_request_name);
	tcase_add_test(tc_con_info_translator,test_info_translator_parse_request_email_webPage);
	tcase_add_test(tc_con_info_translator,test_info_translator_parse_request_businessAdr); 
	tcase_add_test(tc_con_info_translator,test_info_translator_parse_request_pagerNumber); 
	tcase_add_test(tc_con_info_translator,test_info_translator_parse_request_birthday); 
	tcase_add_test(tc_con_info_translator,test_info_translator_parse_request_blank);
	tcase_add_test(tc_con_info_translator,test_info_translator_parse_request_mobile);
	tcase_add_test(tc_con_info_translator,test_info_translator_parse_request_note);

	tcase_add_test (tc_con_info_translator, test_info_translator_parse_response_jobTitle);
	tcase_add_test(tc_con_info_translator,test_info_translator_parse_response_name);
	tcase_add_test(tc_con_info_translator,test_info_translator_parse_response_email_webPage);
	tcase_add_test(tc_con_info_translator,test_info_translator_parse_response_businessAdr);
	tcase_add_test(tc_con_info_translator,test_info_translator_parse_response_otherAdr);
	tcase_add_test(tc_con_info_translator,test_info_translator_parse_response_pagerNumber);
	tcase_add_test(tc_con_info_translator,test_info_translator_parse_response_birthday);
	tcase_add_test(tc_con_info_translator,test_info_translator_parse_response_blank);
	tcase_add_test(tc_con_info_translator,test_info_translator_parse_response_mobile);
	tcase_add_test(tc_con_info_translator,test_info_translator_parse_response_note);
	tcase_add_test(tc_con_info_translator,test_info_translator_parse_response_all);
    
    return s;
}
