/*******************************************************************************
 * Copyright (C) 2004-2006 Intel Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  - Neither the name of Intel Corp. nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL Intel Corp. OR THE CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

/**
 * @author Haihao Xiang
 * @author Anas Nashif
 * @author Eugene Yarmosh
 */

#include "config.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"
#include <gmodule.h>

#include "ws_utilities.h"



#include "ws_errors.h"
#include "ws_xml_api.h"
#include "soap_api.h"
#include "xml_api_generic.h"
#include "xml_serializer.h"
#include "ws_dispatcher.h"

#include "IpmiSel_data.h"
#include "wsman-debug.h"
#include "sfcc-interface.h"
#include "interface_utils.h"

// ******************* IPMISEL *******************************
static IpmiSel *set_key_values(CMPIObjectPath *objpath)
{
	IpmiSel *g_IpmiSel = (IpmiSel *)malloc(sizeof(IpmiSel));
        
        memset(g_IpmiSel, 0, sizeof(IpmiSel));
	g_IpmiSel->LogCreationClassName =  cim_get_keyvalue(objpath, "LogCreationClassName");
	g_IpmiSel->LogName =  cim_get_keyvalue(objpath, "LogName");	
	g_IpmiSel->CreationClassName = cim_get_keyvalue(objpath, "CreationClassName");
	g_IpmiSel->RecordID =  cim_get_keyvalue(objpath, "RecordID");
	g_IpmiSel->MessageTimestamp = cim_get_keyvalue(objpath, "MessageTimestamp");
	g_IpmiSel->RecordData = cim_get_keyvalue(objpath, "RecordData");

        return g_IpmiSel;
}

static IpmiSel *set_values(CMPIInstance *instance)
{
	IpmiSel *g_IpmiSel = (IpmiSel *)malloc(sizeof(IpmiSel));
	
	g_IpmiSel->LogCreationClassName =  cim_get_property(instance, "LogCreationClassName");
	g_IpmiSel->LogName =  cim_get_property(instance, "LogName");	
	g_IpmiSel->CreationClassName = cim_get_property(instance, "CreationClassName");
	g_IpmiSel->RecordID =  cim_get_property(instance, "RecordID");
	g_IpmiSel->MessageTimestamp = cim_get_property(instance, "MessageTimestamp");
	g_IpmiSel->RecordData = cim_get_property(instance, "RecordData");
	
	return g_IpmiSel;
	
}

IpmiSel* IpmiSel_Get_EP(WsContextH cntx)
{
		
	IpmiSel *g_IpmiSel = (IpmiSel *)malloc(sizeof(IpmiSel));	
	wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Get Endpoint Called");
	
	// Keys
	GList *keys = create_key_list(cntx, IpmiSel_Get_Selectors);
	wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "Number of keys defined: %d", g_list_length(keys));
	
	CimClientInfo cimclient; // = malloc(sizeof(CimClientInfo));
	cim_connect_to_cimom(&cimclient, "localhost", NULL, NULL );
	if (!cimclient.cc)
		return NULL;
			
	CMPIInstance *instance = cim_get_instance(cimclient.cc, "IPMISEL", keys);					
	g_IpmiSel = set_values(instance );				
    return g_IpmiSel;
}


int IpmiSel_Enumerate_EP(WsContextH cntx, WsEnumerateInfo* enumInfo)
{
	wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Enumerate Endpoint Called"); 
	
	CimClientInfo cimclient;
	cim_connect_to_cimom(&cimclient, "localhost", NULL, NULL );
	if (!cimclient.cc)
		return 1;
		
    CMPIArray * enumArr = cim_enum_instancenames(cimclient.cc, "IPMI_SEl" );
    if (!enumArr)
	    return 1;

    enumInfo->totalItems = cim_enum_totalItems(enumArr);
    enumInfo->enumResults = enumArr;
    return 0;
}

int IpmiSel_Release_EP(WsContextH cntx, WsEnumerateInfo* enumInfo)
{
	wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Release Endpoint Called");      
        return 0;
}

int IpmiSel_Pull_EP(WsContextH cntx, WsEnumerateInfo* enumInfo)
{ 
	IpmiSel *g_IpmiSel = (IpmiSel *)malloc(sizeof(IpmiSel));
	wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Pull Endpoint Called"); 
	
 	if ( enumInfo->index >= 0 && enumInfo->index < enumInfo->totalItems )
    {
    		CMPIArray * results = (CMPIArray *)enumInfo->enumResults;
    		CMPIData data = results->ft->getElementAt(results, enumInfo->index, NULL);
    		// g_IpmiSel = set_values(data.value.inst);
                g_IpmiSel = set_key_values(data.value.ref);
    		enumInfo->pullResultPtr = g_IpmiSel;
    }
    else
    {    	
    		enumInfo->pullResultPtr = NULL;
    }

    return 0;
}

#define CATCH_RESP      "CatchResponse"
#define PLUGIN_INFO     "This plugin is designed for IPMI_SEL class"
int IpmiSel_Catch_EP(SoapOpH op, void* appData)
{
	wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Catch Endpoint Called"); 

        SoapH soap = soap_get_op_soap(op);	
        WsContextH cntx = ws_create_ep_context(soap, soap_get_op_doc(op, 1));    
        // WsDispatchEndPointInfo* info = (WsDispatchEndPointInfo*)appData;

        // void* data;
        WsXmlDocH doc = NULL;

        // add some code for private endpoint

        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "Creating Response doc");
        doc = ws_create_response_envelope(cntx, soap_get_op_doc(op, 1), NULL);

        if ( doc )
        {
                WsXmlNodeH body = ws_xml_get_soap_body(doc);
                ws_xml_add_child(body, 
                                 XML_NS_DOEM_TEST,
                                 CATCH_RESP,
                                 PLUGIN_INFO);
        }

        if ( doc )
        {
                wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "Setting operation document");
                soap_set_op_doc(op, doc, 0);
                soap_submit_op(op, soap_get_op_channel_id(op), NULL);
                ws_xml_destroy_doc(doc);
        }

        ws_serializer_free_all(cntx);
        return 0;
}
