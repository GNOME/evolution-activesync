#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>
#include <glib.h>
#include <unistd.h>

#include "eas_test_user.h"
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <sys/stat.h>
#include "../libeassync/src/libeassync.h"
#include "../libeassync/src/eas-item-info.h"
#include "../eas-daemon/libeas/eas-cal-info-translator.h"
#include <libebook/e-vcard.h>

//-------------------------------------------------------------//
//   Input Vcard file it should use UTF-8 with CR+LF format   //
//   To do that, you can use on linux  Leafpad 0.8.17          //
//-------------------------------------------------------------//

Suite* eas_cal_info_translator_suite (void);

static void test_eas_cal_info_translator_parse_request(const char* vCalendarName,const char* xmlName)
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
	struct stat stFileInfo;
	EasItemInfo* calInfo = NULL;
	size_t readResult;
	gchar *ptr= NULL;
	long size;
	gchar *buf;
	gboolean parseResponse;

#if !GLIB_CHECK_VERSION(2,36,0)
	g_type_init();
#endif

	calInfo = eas_item_info_new();
	calInfo->server_id = serv;
//endregion
//check the VCalendar file, did the VCalendar file exists
	size = pathconf(".", _PC_PATH_MAX);
	if ((buf = (char *)malloc((size_t)size)) != NULL)
	ptr = getcwd(buf, (size_t)size);
	fail_if(!stat(g_strconcat (ptr, "/TestData/Cal_Info_Translator/_Request/VCalendar_Data/",vCalendarName,NULL),&stFileInfo)==0,"The test file from VCalendar_Data folder does not exist,Please check your VCalendar_Data folder.(check_tests/TestData/Cal_Info_Translator/_Request/VCalendar_Data");
//end checking

//region Load VCard data
	fr = fopen(g_strconcat (ptr, "/TestData/Cal_Info_Translator/_Request/VCalendar_Data/",vCalendarName,NULL), "r");
	fail_if(fr==NULL,"The test file from VCalendar_Data folder does not have good structure.", "Please check your VCalendar_Data folder.(check_tests/TestData/Cal_Info_Translator/_Request/Vcalendar_Data)");
	fseek (fr , 0 , SEEK_END);
	lSize1 = ftell (fr);
	rewind (fr);
	buffer1 = (gchar*) malloc (sizeof(gchar)*lSize1 + 1);
	readResult=fread (buffer1,sizeof(gchar),lSize1,fr);
	fail_if(readResult == 0);
	buffer1[lSize1]='\0';
	calInfo->data = buffer1;
	fclose (fr);
//end Loading

//Load XML Root Node
	doc = xmlParseFile(g_strconcat (ptr, "/TestData/Cal_Info_Translator/_Request/XML_Data/temp.xml",NULL));
//end Loading

//region Translate
	fail_if(doc==NULL,"The test file from XML_Data folder does not have good structure", "Please check your XML_Data folder.(check_tests/TestData/Cal_Info_Translator/XML_Data/");
	nodeLevel1 = doc->children;
	parseResponse = eas_cal_info_translator_parse_request(doc, nodeLevel1, calInfo);
	fail_unless(parseResponse, "XML can't be created.");
//end Translation

//region Save Translation in temp.txt	
		fw = fopen(g_strconcat (ptr, "/TestData/Cal_Info_Translator/_Request/XML_Data/temp.txt",NULL), "w");
		xmlDocFormatDump(fw,doc,1);
	fclose(fw);
	xmlFreeDoc(doc);
//end region

//region Load Translation
fr = fopen(g_strconcat (ptr, "/TestData/Cal_Info_Translator/_Request/XML_Data/temp.txt",NULL), "r");
	fail_if(fr==NULL,"The temp file from XML_Data folder does not have good structure.", "Please check your XML_Data folder.(check_tests/TestData/Cal_Info_Translator/_Request/XML_Data)");
	fseek (fr , 0 , SEEK_END);
	lSize2 = ftell (fr);
	rewind (fr);
	buffer2 = (gchar*) malloc (sizeof(gchar)*lSize2 + 1);
	readResult=fread (buffer2,sizeof(gchar),lSize2,fr);
	fail_if(readResult == 0);
	buffer2[lSize2]='\0';
	fclose (fr);
	remove(g_strconcat (ptr, "/TestData/Cal_Info_Translator/_Request/XML_Data/temp.txt",NULL)); //Delete the temporary File temp.txt
//end Loading Translation

//region Load XML Data 
fr = fopen(g_strconcat (ptr, "/TestData/Cal_Info_Translator/_Request/XML_Data/",xmlName,NULL), "r");
	fail_if(fr==NULL,"The test file from XML_Data folder does not have good structure.", "Please check your XML_Data folder.(check_tests/TestData/Cal_Info_Translator/_Request/XML_Data)");
	fseek (fr , 0 , SEEK_END);
	lSize3 = ftell (fr);
	rewind (fr);
	buffer3 = (gchar*) malloc (sizeof(gchar)*lSize3 + 1);
	readResult=fread (buffer3,sizeof(gchar),lSize3,fr);
	fail_if(readResult == 0);
	buffer3[lSize3]='\0';
	
	fclose (fr);

	fail_if(g_strcmp0 (buffer2,buffer3)!=0, "The VCalendar file it was not properly translated. Please check input data. In other case, function does not work properly.");

}

START_TEST(test_eas_cal_info_translator_parse_request_startTime)
{
test_eas_cal_info_translator_parse_request("VCard_cal_info_translator_parse_request_startTime.txt","_XML_cal_info_translator_parse_request_startTime.xml");
}
END_TEST

START_TEST(test_eas_cal_info_translator_parse_request_attendee)
{
test_eas_cal_info_translator_parse_request("VCard_cal_info_translator_parse_request_attendee.txt","_XML_cal_info_translator_parse_request_attendee.xml");
}
END_TEST

START_TEST(test_eas_cal_info_translator_parse_request_timeZone)
{
test_eas_cal_info_translator_parse_request("VCard_cal_info_translator_parse_request_timeZone.txt","_XML_cal_info_translator_parse_request_timeZone.xml");
}
END_TEST

START_TEST(test_eas_cal_info_translator_parse_request_organizer)
{
test_eas_cal_info_translator_parse_request("VCard_cal_info_translator_parse_request_organizer.txt","_XML_cal_info_translator_parse_request_organizer.xml");
}
END_TEST

START_TEST(test_eas_cal_info_translator_parse_request_allDayEvent)
{
test_eas_cal_info_translator_parse_request("VCard_cal_info_translator_parse_request_allDayEvent.txt","_XML_cal_info_translator_parse_request_allDayEvent.xml");
}
END_TEST

START_TEST(test_eas_cal_info_translator_parse_request_body)
{
test_eas_cal_info_translator_parse_request("VCard_cal_info_translator_parse_request_body.txt","_XML_cal_info_translator_parse_request_body.xml");
}
END_TEST

START_TEST(test_eas_cal_info_translator_parse_request_busy)
{
test_eas_cal_info_translator_parse_request("VCard_cal_info_translator_parse_request_busy.txt","_XML_cal_info_translator_parse_request_busy.xml");
}
END_TEST

START_TEST(test_eas_cal_info_translator_parse_request_location)
{
test_eas_cal_info_translator_parse_request("VCard_cal_info_translator_parse_request_location.txt","_XML_cal_info_translator_parse_request_location.xml");
}
END_TEST

START_TEST(test_eas_cal_info_translator_parse_request_reminder)
{
test_eas_cal_info_translator_parse_request("VCard_cal_info_translator_parse_request_reminder.txt","_XML_cal_info_translator_parse_request_reminder.xml");
}
END_TEST

START_TEST(test_eas_cal_info_translator_parse_request_subject)
{
test_eas_cal_info_translator_parse_request("VCard_cal_info_translator_parse_request_subject.txt","_XML_cal_info_translator_parse_request_subject.xml");
}
END_TEST

START_TEST(test_eas_cal_info_translator_parse_request_uid)
{
test_eas_cal_info_translator_parse_request("VCard_cal_info_translator_parse_request_uid.txt","_XML_cal_info_translator_parse_request_uid.xml");
}
END_TEST

START_TEST(test_eas_cal_info_translator_parse_request_recurrence)
{
test_eas_cal_info_translator_parse_request("VCard_cal_info_translator_parse_request_recurrence.txt","_XML_cal_info_translator_parse_request_recurrence.xml");
}
END_TEST

START_TEST(test_eas_cal_info_translator_parse_request_exception)
{
test_eas_cal_info_translator_parse_request("VCard_cal_info_translator_parse_request_exception.txt","_XML_cal_info_translator_parse_request_exception.xml");
}
END_TEST

START_TEST(test_eas_cal_info_translator_parse_request_category)
{
test_eas_cal_info_translator_parse_request("VCard_cal_info_translator_parse_request_category.txt","_XML_cal_info_translator_parse_request_category.xml");
}
END_TEST
static void test_eas_cal_info_translator_parse_response(const char* vcardName, const char* xmlName)
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
//check the VCalendar file, did the VCalendar file exists

	gchar *ptr= NULL;
	long size;
	gchar *buf;
	size = pathconf(".", _PC_PATH_MAX);
	if ((buf = (char *)malloc((size_t)size)) != NULL)
	ptr = getcwd(buf, (size_t)size);

	fail_if(!stat(g_strconcat (ptr, "/TestData/Cal_Info_Translator/_Response/VCalendar_Data/",vcardName,NULL),&stFileInfo)==0,"The test file from VCalendar_Data folder does not exist,Please check your VCalendar_Data folder.(check_tests/TestData/Cal_Info_Translator/VCalendar_Data");
//end checking

//region Load VCard data
	fr = fopen(g_strconcat (ptr, "/TestData/Cal_Info_Translator/_Response/VCalendar_Data/",vcardName,NULL), "r");
	fail_if(fr==NULL,"The test file from VCalendar_Data folder does not have good structure.", "Please check your VCalendar_Data folder.(check_tests/TestData/Cal_Info_Translator/_Response/VCalendar_Data)");
	
	fseek (fr , 0 , SEEK_END);
	lSize = ftell (fr);
	rewind (fr);
	buffer = (gchar*) malloc (sizeof(gchar)*lSize+1);
	readResult=fread (buffer,sizeof(gchar),lSize,fr);
	fail_if(readResult == 0);
	buffer[lSize]='\0';
//endregion

//check the xml file, did the xml file exists
	fail_if(!stat(g_strconcat (ptr, "/TestData/Cal_Info_Translator/_Response/XML_Data/",xmlName,NULL),&stFileInfo)==0,"The test file from XML_Data folder does not exist,Please check your XML_Data folder.(check_tests/TestData/Cal_Info_Translator/_Response/XML_Data");
//end checking

//region Load XML test data
	doc = xmlParseFile(g_strconcat (ptr, "/TestData/Cal_Info_Translator/_Response/XML_Data/",xmlName,NULL));
//endregion

	fail_if(doc==NULL,"The test file from XML_Data folder does not have good structure", "Please check your XML_Data folder.(check_tests/TestData/Cal_Info_Translator/_Response/XML_Data");

	nodeLevel1 = doc->children;
	g_type_init();
	fail_if(nodeLevel1==NULL,"The XML file does not have correct structure");

	res = eas_cal_info_translator_parse_response(nodeLevel1, serv);
	fclose (fr);

	item = eas_item_info_new();
	eas_item_info_deserialise(item,res);

	fail_if(g_strcmp0 (buffer,item->data)!=0, "The XML file it was not properly translated. Please check input data. In other case, function does not work properly.");
	

	xmlFreeDoc(doc);

}
START_TEST(test_eas_cal_info_translator_parse_response_startTime)
{
test_eas_cal_info_translator_parse_response("VCard_cal_info_translator_parse_response_startTime.txt","_XML_cal_info_translator_parse_response_startTime.xml");
}
END_TEST
START_TEST(test_eas_cal_info_translator_parse_response_timeZone)
{
test_eas_cal_info_translator_parse_response("VCard_cal_info_translator_parse_response_timeZone.txt","_XML_cal_info_translator_parse_response_timeZone.xml");
}
END_TEST

START_TEST(test_eas_cal_info_translator_parse_response_organizer)
{
test_eas_cal_info_translator_parse_response("VCard_cal_info_translator_parse_response_organizer.txt","_XML_cal_info_translator_parse_response_organizer.xml");
}
END_TEST

START_TEST(test_eas_cal_info_translator_parse_response_allDayEvent)
{
test_eas_cal_info_translator_parse_response("VCard_cal_info_translator_parse_response_allDayEvent.txt","_XML_cal_info_translator_parse_response_allDayEvent.xml");
}
END_TEST

START_TEST(test_eas_cal_info_translator_parse_response_body)
{
test_eas_cal_info_translator_parse_response("VCard_cal_info_translator_parse_response_body.txt","_XML_cal_info_translator_parse_response_body.xml");
}
END_TEST

START_TEST(test_eas_cal_info_translator_parse_response_busy)
{
test_eas_cal_info_translator_parse_response("VCard_cal_info_translator_parse_response_busy.txt","_XML_cal_info_translator_parse_response_busy.xml");
}
END_TEST
START_TEST(test_eas_cal_info_translator_parse_response_location)
{
test_eas_cal_info_translator_parse_response("VCard_cal_info_translator_parse_response_location.txt","_XML_cal_info_translator_parse_response_location.xml");
}
END_TEST

START_TEST(test_eas_cal_info_translator_parse_response_reminder)
{
test_eas_cal_info_translator_parse_response("VCard_cal_info_translator_parse_response_reminder.txt","_XML_cal_info_translator_parse_response_reminder.xml");
}
END_TEST

/*START_TEST(test_eas_cal_info_translator_parse_response_sensitivity)
{
test_eas_cal_info_translator_parse_response("VCard_cal_info_translator_parse_response_sensitivity.txt","_XML_cal_info_translator_parse_response_sensitivity.xml");
}
END_TEST*/
START_TEST(test_eas_cal_info_translator_parse_response_subject)
{
test_eas_cal_info_translator_parse_response("VCard_cal_info_translator_parse_response_subject.txt","_XML_cal_info_translator_parse_response_subject.xml");
}
END_TEST
START_TEST(test_eas_cal_info_translator_parse_response_uid)
{
test_eas_cal_info_translator_parse_response("VCard_cal_info_translator_parse_response_uid.txt","_XML_cal_info_translator_parse_response_uid.xml");
}
END_TEST
START_TEST(test_eas_cal_info_translator_parse_response_attendee)
{
test_eas_cal_info_translator_parse_response("VCard_cal_info_translator_parse_response_attendee.txt","_XML_cal_info_translator_parse_response_attendee.xml");
}
END_TEST
START_TEST(test_eas_cal_info_translator_parse_response_recurrence)
{
test_eas_cal_info_translator_parse_response("VCard_cal_info_translator_parse_response_recurrence.txt","_XML_cal_info_translator_parse_response_recurrence.xml");
}
END_TEST

START_TEST(test_eas_cal_info_translator_parse_response_exception)
{
test_eas_cal_info_translator_parse_response("VCard_cal_info_translator_parse_response_exception.txt","_XML_cal_info_translator_parse_response_exception.xml");
}
END_TEST

START_TEST(test_eas_cal_info_translator_parse_response_category)
{
test_eas_cal_info_translator_parse_response("VCard_cal_info_translator_parse_response_category.txt","_XML_cal_info_translator_parse_response_category.xml");
}
END_TEST


Suite* eas_cal_info_translator_suite (void)
{
    Suite* s = suite_create ("con_info_translator");

    	/* col-info-translator test case */
    	TCase *tc_cal_info_translator = tcase_create ("core");
    	suite_add_tcase (s, tc_cal_info_translator);

   	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_request_startTime);
	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_request_timeZone);
	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_request_organizer);
	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_request_allDayEvent); 
	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_request_body);
	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_request_busy);
	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_request_location);
   	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_request_reminder);
	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_request_subject);
	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_request_uid);
	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_request_attendee);
//	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_request_recurrence);//test will fail because activesync does not 	
													//support WKST attribute of RRULE property.
	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_request_exception);
	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_request_category); 

	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_response_startTime);
     	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_response_timeZone);
	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_response_organizer);
	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_response_allDayEvent);
	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_response_body);
	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_response_busy);
	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_response_location);
	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_response_reminder);
	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_response_subject);
	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_response_uid);
	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_response_attendee);
	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_response_recurrence); 

	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_response_exception); 
 	 tcase_add_test (tc_cal_info_translator, test_eas_cal_info_translator_parse_response_category);	
		
    return s;
}
