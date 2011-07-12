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
#include <libebook/e-vcard.h>

static void test_eas_con_info_translator_parse_response(xmlNodePtr app_data, const gchar* server_id)
{
	xmlNodePtr XMLdata = app_data;
	fail_if (XMLdata == NULL,
             "XMLdata equals NULL!");

	const gchar* serv = server_id;
	fail_if (serv == NULL,
             "Server_id equals NULL!");

	gchar* ret = eas_con_info_translator_parse_response(XMLdata, serv);
	mark_point();

	fail_if(ret ==NULL,
		"Vcard is empty");
}


static void test_eas_con_info_translator_parse_request(xmlDocPtr doc,
                                               xmlNodePtr app_data,
                                               EasItemInfo* cal_info)
{
    //ToDo create test over here
}

//-------------------------------------------------------------//
//   Input Vcard file it shoulde use UTF-8 with CR+LF format   //
//   To do that, you can use on linux  Leafpad 0.8.17          //
//-------------------------------------------------------------//

START_TEST (test_info_translator_parse_response_jobtitle)
{ 
//region init variable
	xmlDocPtr doc;
	xmlNodePtr nodeLevel1;
	FILE *fr;
	gchar temp[]="3:1";
	gchar* serv=temp;
	gchar* res = NULL;
	long lSize;
	gchar * buffer=NULL;
	struct stat stFileInfo;
//endregion
//check the Vdata file, did the Vdata file exists
	fail_if(!stat(g_strconcat (getenv ("HOME"), "/activesyncd/check_tests/TestData/Con_Info_Translator/VCard_Data/VCard_info_translator_parse_response_JobTitle.txt",NULL),&stFileInfo)==0,"The test file from VData_Data folder does not exist,Please check your Vdata_Data folder.(check_tests/TestData/Con_Info_Translator/VCard_Data");
//end checking

//region Load VCard data
	fr = fopen(g_strconcat (getenv ("HOME"), "/activesyncd/check_tests/TestData/Con_Info_Translator/VCard_Data/VCard_info_translator_parse_response_JobTitle.txt",NULL), "r");
	fail_if(fr==NULL,"The test file from VCard_Data folder does not have good structure.", "Please check your VCard_Data folder.(check_tests/TestData/Con_Info_Translator/VCard_Data)");

	fseek (fr , 0 , SEEK_END);
	lSize = ftell (fr);
	rewind (fr);
	buffer = (gchar*) malloc (sizeof(gchar)*lSize+1);
	fread (buffer,sizeof(gchar),lSize,fr);
	buffer[lSize]=NULL;
	//endregion

//check the xml file, did the xml file exists
	fail_if(!stat(g_strconcat (getenv ("HOME"), "/activesyncd/check_tests/TestData/Con_Info_Translator/XML_Data/_XML_info_translator_parse_response_JobTitle.xml",NULL),&stFileInfo)==0,"The test file from XML_Data folder does not exist,Please check your XML_Data folder.(check_tests/TestData/Con_Info_Translator/XML_Data");
//end checking

//region Load XML test data
	doc = xmlParseFile(g_strconcat (getenv ("HOME"), "/activesyncd/check_tests/TestData/Con_Info_Translator/XML_Data/_XML_info_translator_parse_response_JobTitle.xml",NULL));
//endregion

	fail_if(doc==NULL,"The test file from XML_Data folder does not have good structure", "Please check your XML_Data folder.(check_tests/TestData/Con_Info_Translator/XML_Data");

	nodeLevel1 = doc->children;
	g_type_init();
	fail_if(nodeLevel1==NULL,"The XML file does not have correct structure");

	res = eas_con_info_translator_parse_response(nodeLevel1, serv);
	fclose (fr);
	
	fail_if(g_strcmp0 (buffer,res)!=0, "The XML file it was not properly translated. Please check input data. In other case, function does not work properly.");
	
	xmlFreeDoc(doc);
}
END_TEST


Suite* eas_con_info_translator_suite (void)
{
    Suite* s = suite_create ("con_info_translator");

    /* con-info-translator test case */
    TCase *tc_con_info_translator = tcase_create ("core");
    suite_add_tcase (s, tc_con_info_translator);
    tcase_add_test (tc_con_info_translator, test_info_translator_parse_response_jobtitle);

    return s;
}
