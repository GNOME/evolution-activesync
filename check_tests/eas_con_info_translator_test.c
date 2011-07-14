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
void test_info_translator_parse_response(char* vcardName, char* xmlName)
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
//endregion
//check the VCard file, did the VCard file exists

	gchar *ptr= NULL;
	long size;
	gchar *buf;
	size = pathconf(".", _PC_PATH_MAX);
	if ((buf = (char *)malloc((size_t)size)) != NULL)
	ptr = getcwd(buf, (size_t)size);

	fail_if(!stat(g_strconcat (ptr, "/TestData/Con_Info_Translator/VCard_Data/",vcardName,NULL),&stFileInfo)==0,"The test file from VCard_Data folder does not exist,Please check your VCard_Data folder.(check_tests/TestData/Con_Info_Translator/VCard_Data");
//end checking

//region Load VCard data
	fr = fopen(g_strconcat (ptr, "/TestData/Con_Info_Translator/VCard_Data/",vcardName,NULL), "r");
	fail_if(fr==NULL,"The test file from VCard_Data folder does not have good structure.", "Please check your VCard_Data folder.(check_tests/TestData/Con_Info_Translator/VCard_Data)");
	
	gchar* temporarybuffer=NULL;
	fseek (fr , 0 , SEEK_END);
	lSize = ftell (fr);
	rewind (fr);
	temporarybuffer = (gchar*) malloc (sizeof(gchar)*lSize+1);
	fread (temporarybuffer,sizeof(gchar),lSize,fr);
	temporarybuffer[lSize]='\0';
	
	char headofVcard[]={(char)10,temp[0],temp[1],temp[2],(char)10,NULL};
	buffer = (gchar *)malloc(sizeof(gchar)*lSize+2);
	buffer=g_strconcat (headofVcard, temporarybuffer,NULL);
	//endregion

//check the xml file, did the xml file exists
	fail_if(!stat(g_strconcat (ptr, "/TestData/Con_Info_Translator/XML_Data/",xmlName,NULL),&stFileInfo)==0,"The test file from XML_Data folder does not exist,Please check your XML_Data folder.(check_tests/TestData/Con_Info_Translator/XML_Data");
//end checking

//region Load XML test data
	doc = xmlParseFile(g_strconcat (ptr, "/TestData/Con_Info_Translator/XML_Data/",xmlName,NULL));
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
