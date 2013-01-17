#include <stdio.h>
#include <stdlib.h>
#define ASN1_HAS_IOSTREAM
#include <sstream>
#include <iostream>
#include <asn1.h>
#include <multimedia_system_control.h>
extern "C"{
#include "h245templates.h"
#include "h324m.h"
}


#define UNTIL_CLOSING_FLAG (-1)
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif



/*	printh245 takes a H245 buffer PER encoded and print it out
*
*
*		
*/
extern "C"
void printh245(char *buf, int len)
{
	std::stringstream mybuf;
	MULTIMEDIA_SYSTEM_CONTROL::MultimediaSystemControlMessage msg;
	ASN1::PERDecoder decoder(buf,buf+len);
	ASN1::AVNEncoder encoder(mybuf);

	while(msg.accept(decoder)){
		msg.accept(encoder);
		decoder.setPosition(decoder.getNextPosition());
	}
	std::cerr << mybuf.str() << std::endl;
}









size_t H245EncodeTemplate(const char *inbuf, unsigned  char* outbuf)
{
	char tmpout[1024];
	std::vector<char> encbuf;

	MULTIMEDIA_SYSTEM_CONTROL::MultimediaSystemControlMessage msg;
	ASN1::PEREncoder pencoder(encbuf);
	ASN1::CoderEnv env;
	env.set_encodingRule(ASN1::CoderEnv::avn);

	ASN1::decode(inbuf, inbuf + strlen(inbuf), &env, msg);
	msg.accept(pencoder);

	unsigned int i = 0;
	for(std::vector<char>::const_iterator encit = encbuf.begin(); encit < encbuf.end(); encit ++)
	{
		tmpout[i] = (char)(*encit);
		i++;
	}	

	memcpy(outbuf, tmpout, i);
	return i;
}





/*	getLocalH245* makes the PER encoded buffer of the specific form	
*
*
*		
*/
extern "C"
size_t getLocalH245TerminalCapabilitySet (h324m_session *session, unsigned char* buf)
{
	char tmpbuf [4096];
	size_t size;

	sprintf(tmpbuf, template_TerminalCapabilitySet, 1);
	size = H245EncodeTemplate(tmpbuf,buf);

	return size;
}


size_t getLocalH245TerminalCapabilitySetAck (h324m_session *session, unsigned int seqn, unsigned char* buf)
{

	char tmpbuf [4096];
	int size = sprintf(tmpbuf, template_TerminalCapabilitySetAck, seqn);
	return H245EncodeTemplate (tmpbuf, buf);
}



size_t getLocalH245RoundTripleDelayResponse (h324m_session *session, unsigned int seqn, unsigned char* buf)
{
	char tmpbuf [4096];
	sprintf(tmpbuf, template_roundTripleDelayResponse, seqn);
	return H245EncodeTemplate (tmpbuf, buf);
}



size_t getLocalH245VendorId (h324m_session *session, unsigned char* buf)
{
	unsigned char codedbuf[255];
	std::vector<char> encbuf;
	MULTIMEDIA_SYSTEM_CONTROL::MultimediaSystemControlMessage msg;
	MULTIMEDIA_SYSTEM_CONTROL::VendorIdentification vendor;

	ASN1::PEREncoder pencoder(encbuf);
	vendor.set_productNumber() = "CVF";
	vendor.set_versionNumber() = "2.0";
	vendor.set_vendor().select_h221NonStandard().set_manufacturerCode(0);
	vendor.ref_vendor().select_h221NonStandard().set_t35Extension(0);
	vendor.ref_vendor().select_h221NonStandard().set_t35CountryCode(33);
	msg.select_indication().select_vendorIdentification() = vendor;
	msg.accept(pencoder);
	int i = 0;
	for(std::vector<char>::const_iterator encit = encbuf.begin(); encit < encbuf.end(); encit++){
		codedbuf[i] = (unsigned char)(*encit);
		i++;
	}
	memcpy(buf, codedbuf, i);
	printf("VendorId Send with len %d\n",i);
	return i;
}


size_t getLocalH245MasterSlaveDetermination (h324m_session *session, unsigned char* buf, unsigned int termtype,unsigned int sdnumber)
{
	char tmpbuf [4096];   
	size_t size = 0;
	size = sprintf(tmpbuf, template_MasterSlaveDetermination, termtype, sdnumber);
	return H245EncodeTemplate (tmpbuf, buf);
}


size_t getLocalH245MasterSlaveDeterminationAck (h324m_session *session, unsigned char* buf, unsigned int isMaster)
{
	char tmpbuf [4096];
	int size = 0;
	if (isMaster == 1){
		size = sprintf(tmpbuf, template_MasterSlaveDeterminationAckMaster);
	}
	else{
		size = sprintf(tmpbuf, template_MasterSlaveDeterminationAckSlave);
	}
	return H245EncodeTemplate (tmpbuf, buf);
}


size_t getLocalH245MasterSlaveDeterminationAck (h324m_session *session, unsigned char* buf, unsigned int termtype, unsigned int sdnumber)
{

	fprintf (stderr, "getLocalH245MasterSlaveDeterminationAck, termtype=%d, sdnumber=%d\n", termtype, sdnumber);
	char tmpbuf [4096];

	int size = 0;

	if (session->TerminalType > termtype){
		session->MasterSlave = SLAVE;
		size = sprintf(tmpbuf, template_MasterSlaveDeterminationAckSlave);
		fprintf (stderr, "MSD ack Slave is \n%s\nsize is %d\n", tmpbuf, size);
		return H245EncodeTemplate (tmpbuf, buf);
	}

	if (session->TerminalType < termtype)	{
		session->MasterSlave = MASTER;
		size = sprintf(tmpbuf, template_MasterSlaveDeterminationAckMaster);
		fprintf (stderr, "MSD ack Master is \n%s\nsize is %d\n", tmpbuf, size);
		return H245EncodeTemplate (tmpbuf, buf);
	}

	if (session->StatusDeterminationNumber  > sdnumber)	{
		session->MasterSlave = SLAVE;
		size = sprintf(tmpbuf, template_MasterSlaveDeterminationAckSlave);
		fprintf (stderr, "MSD ack Slave is \n%s\nsize is %d\n", tmpbuf, size);
		return H245EncodeTemplate (tmpbuf, buf);
	}

	if (session->StatusDeterminationNumber  < sdnumber)	{	
		session->MasterSlave = MASTER;
		size = sprintf(tmpbuf, template_MasterSlaveDeterminationAckMaster);
		fprintf (stderr, "MSD ack Master is \n%s\nsize is %d\n", tmpbuf, size);
		return H245EncodeTemplate (tmpbuf, buf);
	}
	return 0;
}

size_t getLocalH245OpenLogicalChannelAudio (h324m_session *session,  unsigned char* buf)
{
	size_t size = 0;
	size = H245EncodeTemplate (template_OpenLogicalChannelAMR, buf);
	return size;
}


size_t getLocalH245OpenLogicalChannelVideo (h324m_session *session,  unsigned char* buf)
{
	size_t size = 0;
	size += H245EncodeTemplate (template_OpenLogicalChannelH263, buf);
	return size;
}

size_t getLocalH245OpenLogicalChannelAck (h324m_session *session, unsigned char* buf, unsigned int channel)
{
	char tmpbuf [4096];
	sprintf(tmpbuf, template_OpenLogicalChannelAck, channel);
	return H245EncodeTemplate (tmpbuf, buf);
}

size_t getLocalH245CloseLogicalChannelAck (h324m_session *session, unsigned char* buf, unsigned int channel)
{
	char tmpbuf [4096];
	sprintf(tmpbuf, template_CloseLogicalChannelAck, channel);
	return H245EncodeTemplate (tmpbuf, buf);
}

size_t getLocalH245MultiplexEntrySend (h324m_session *session, unsigned char* buf)
{	
	return H245EncodeTemplate (template_MultiplexEntrySend, buf);
}


size_t getLocalH245VideoFastUpdate (h324m_session *session, unsigned char* buf)
{	
	return H245EncodeTemplate (template_VideoFastUpdate, buf);
}


size_t getLocalH245MultiplexEntrySendAck (h324m_session *session,unsigned char* buf, unsigned int seqn)
{
	char tmpbuf [4096];
	sprintf(tmpbuf, template_multiplexEntrySendAck, seqn);
	return H245EncodeTemplate (tmpbuf, buf);
}






extern "C"
size_t h245_MessageFromStateMachine(h324m_session *session, unsigned char* buf)
{
	size_t size = 0;
	if(!(session->StateMachine & STATE_TERMCAP)){
		size += getLocalH245TerminalCapabilitySet(session, buf + size);
	}
	if(!(session->StateMachine & STATE_MASTERSLAVE_DETERMINATION)){
		size += getLocalH245MasterSlaveDetermination(session, buf + size,session->TerminalType, session->StatusDeterminationNumber);
	}
	if((session->StateMachine & STATE_TERMCAP) && (session->StateMachine & STATE_MASTERSLAVE_DETERMINATION)){
		if(!(session->StateMachine & STATE_OLC_AUDIO)){
			size += getLocalH245OpenLogicalChannelAudio(session, buf + size);
		}
		if(!(session->StateMachine & STATE_OLC_VIDEO)){
			size += getLocalH245OpenLogicalChannelVideo(session, buf + size);
		}
		if(!(session->StateMachine & STATE_MULTIPLEX_ENTRY_SEND)){
			size += getLocalH245MultiplexEntrySend(session, buf + size);
		}

	}
	return size;

}



/* DestroyMultiplex takes a multiplex element and free the sub/next elements
*
*		
*/
extern "C"
int DestroyMultiplex(MxElement *elem, int isdestroyable)
{
	int i;
	if (elem->nextelement)
		DestroyMultiplex(elem->nextelement,1);
	if (elem->subElementCount)
		for(i = 0; i < elem->subElementCount; i++)
			DestroyMultiplex(elem->subelement[i],1);
	if (isdestroyable)
		free(elem);
	return 0;
}





/* AddElement malloc the Multiplex Element, called when MultiplexEntrySend is called	
*
*		
*/
int AddElement(const MULTIMEDIA_SYSTEM_CONTROL::MultiplexElement element, MxElement* elem)
{
	// We have both "type" and "repeatCount" members

	// Deals with "repeatCount" which can be "finite" or "untilClosingFlag"
	const MULTIMEDIA_SYSTEM_CONTROL::MultiplexElement_repeatCount repeatCount = element.get_repeatCount ();
	bool repeatCountBool = false;
	bool untilClosingFlagBool = false;
	if (repeatCount.finite_isSelected ())
	{
		unsigned int repeatCountValue = repeatCount.get_finite ().getValue ();
		elem->RC = repeatCountValue;
		repeatCountBool = true;
	}
	if (repeatCount.untilClosingFlag_isSelected ())
	{
		elem->RC = UNTIL_CLOSING_FLAG;
		untilClosingFlagBool = true;
	}
	if (repeatCountBool && untilClosingFlagBool)
	{	
		std::cerr << "Both repeatCount and untilClosingFlag are set !" << std::endl;
		return -1;
	}

	// Deals with "type" which can be "logicalChannelNumber" or "subElementList"
	const MULTIMEDIA_SYSTEM_CONTROL::MultiplexElement_type type = element.get_type ();
	if (type.logicalChannelNumber_isSelected ())
	{
		unsigned int lcn = type.get_logicalChannelNumber ().getValue ();
		elem->LCN = lcn;
	}

	if (type.subElementList_isSelected ())
	{
		// a bit of recursion
		MULTIMEDIA_SYSTEM_CONTROL::MultiplexElement_type::subElementList::const_reference subElementList = type.get_subElementList ();
		for (MULTIMEDIA_SYSTEM_CONTROL::MultiplexElement_type::subElementList::value_type::const_iterator elementListItr = subElementList.begin (); elementListItr < subElementList.end (); elementListItr ++)
		{
			const MULTIMEDIA_SYSTEM_CONTROL::MultiplexElement element = (*elementListItr);
			elem->subelement = (MxElement**)realloc (elem->subelement, elem->subElementCount * sizeof(MxElement*));
			elem->subelement[elem->subElementCount] = (MxElement *)malloc(sizeof(MxElement));
			MxElement* currentElement = elem->subelement[elem->subElementCount];
			elem->subElementCount ++;
			currentElement->nextelement = NULL;
			currentElement->subelement = NULL;
			currentElement->subElementCount = 0;
			AddElement (element, currentElement);
		} 
	}
	return 0;
}







int AddMultiplexEntryDescriptor(const MULTIMEDIA_SYSTEM_CONTROL::MultiplexEntryDescriptor* descriptor, MxElement *elem)
{
	const MULTIMEDIA_SYSTEM_CONTROL::MultiplexTableEntryNumber entryNumber = descriptor->get_multiplexTableEntryNumber ();
	unsigned int entryNumberInt = entryNumber.getValue ();



	MxElement* currentElement = NULL;
	if(!descriptor->elementList_isPresent()){
		return 0;
	}
	MULTIMEDIA_SYSTEM_CONTROL::MultiplexEntryDescriptor::elementList::const_reference elementList = descriptor->get_elementList ();

	// Iterate through all the elementList entries
	for (MULTIMEDIA_SYSTEM_CONTROL::MultiplexEntryDescriptor::elementList::value_type::const_iterator elementListItr = elementList.begin (); elementListItr < elementList.end (); elementListItr ++)
	{
		const MULTIMEDIA_SYSTEM_CONTROL::MultiplexElement element = (*elementListItr);
		if (currentElement == NULL)
		{
			currentElement = &elem [entryNumberInt];
			DestroyMultiplex(currentElement,0);
		}
		else
		{
			currentElement->nextelement = (MxElement *)malloc(sizeof(MxElement));
			currentElement = currentElement->nextelement;
		}
		currentElement->nextelement = NULL;
		currentElement->subelement = NULL;
		currentElement->subElementCount = 0;

		AddElement (element, currentElement);
	}
	return 0;
}






int AddMultiplexEntrySendMessage(const MULTIMEDIA_SYSTEM_CONTROL::RequestMessage* msg, h324m_session *session)
{
	MxElement *elem = session->MultiPlexEntryTable;

	if(!msg->multiplexEntrySend_isSelected())
	{
		std::cout << "Buffer is not of type multiplexEntrySend" << std::endl;
		return -1;
	}

	const MULTIMEDIA_SYSTEM_CONTROL::MultiplexEntrySend multiplexEntrySend = msg->get_multiplexEntrySend ();
	unsigned int seqn = multiplexEntrySend.get_sequenceNumber ().getValue();

	MULTIMEDIA_SYSTEM_CONTROL::MultiplexEntrySend::multiplexEntryDescriptors::const_reference descriptors = multiplexEntrySend.get_multiplexEntryDescriptors ();

	// Iterate through all the multiplexEntryDescriptors entries                                                        
	for (MULTIMEDIA_SYSTEM_CONTROL::MultiplexEntrySend::multiplexEntryDescriptors::value_type::const_iterator itr = descriptors.begin (); itr < descriptors.end (); itr ++)
	{
		const MULTIMEDIA_SYSTEM_CONTROL::MultiplexEntryDescriptor descriptor = (*itr);
		AddMultiplexEntryDescriptor (&descriptor, elem);
	}

	return 0;
}



size_t getH245MessageFromRequest (h324m_session *session, const MULTIMEDIA_SYSTEM_CONTROL::RequestMessage* msg, unsigned char* buf)
{
	size_t size = 0;
	unsigned int n;

	/* TCS */
	if (msg->terminalCapabilitySet_isSelected())
	{
		n = msg->get_terminalCapabilitySet().get_sequenceNumber().getValue();
		return getLocalH245TerminalCapabilitySetAck(session,n,buf);
	}

	/* MSD */
	if(msg->masterSlaveDetermination_isSelected())
	{
		return getLocalH245MasterSlaveDeterminationAck(session,buf,0);
	}


	/* OLC */
	if (msg->openLogicalChannel_isSelected())
	{
		n = msg->get_openLogicalChannel().get_forwardLogicalChannelNumber().getValue();
		session->lcntable[n].ALTransport = h324m_AL2;
		if(msg->get_openLogicalChannel().get_forwardLogicalChannelParameters().get_multiplexParameters().get_h223LogicalChannelParameters().get_adaptationLayerType().al2WithSequenceNumbers_isSelected()){
			//h324m_SetAl2(session,n);
			
			session->lcntable[n].type = session->lcntable[n].type | HasSeqNum;
		}
		if(msg->get_openLogicalChannel().get_forwardLogicalChannelParameters().get_multiplexParameters().get_h223LogicalChannelParameters().get_segmentableFlag()){
			session->lcntable[n].type = session->lcntable[n].type | SegmentableFlag;
		}
		if(msg->get_openLogicalChannel().get_forwardLogicalChannelParameters().get_dataType().audioData_isSelected()){
			session->lcntable[n].ALUser = h324m_Amr;
		}
		//Todo: add MPEG4-2
		if(msg->get_openLogicalChannel().get_forwardLogicalChannelParameters().get_dataType().videoData_isSelected()){
			session->lcntable[n].ALUser = h324m_h263;
		}
		
		return getLocalH245OpenLogicalChannelAck(session,buf,n);
	}

	/* MT */
	if(msg->multiplexEntrySend_isSelected())
	{
		AddMultiplexEntrySendMessage(msg,session);
		n = msg->get_multiplexEntrySend().get_sequenceNumber().getValue();
		return getLocalH245MultiplexEntrySendAck(session,buf,n);
	}



	/* Round Trip Delay */
	if (msg->roundTripDelayRequest_isSelected())
	{
		n = msg->get_roundTripDelayRequest().get_sequenceNumber().getValue();
		return getLocalH245RoundTripleDelayResponse(session,n,buf);
	}

	/* Close Logical Channel */
	if (msg->closeLogicalChannel_isSelected())
	{
		n = msg->get_closeLogicalChannel().get_forwardLogicalChannelNumber().getValue();
		return getLocalH245CloseLogicalChannelAck(session,buf,n);
	}

	/* Request Channel Close */
	if (msg->requestChannelClose_isSelected())
	{
	}

	/* Request mode */
	if (msg->requestMode_isSelected())
	{
	}

	/* Maintenance Loop */
	if (msg->maintenanceLoopRequest_isSelected())
	{
	}

	/* Communication Mode */
	if (msg->communicationModeRequest_isSelected())
	{
	}

	/* Conference Request */
	if (msg->conferenceRequest_isSelected())
	{
	}

	/* Multilink Request */
	if (msg->multilinkRequest_isSelected())
	{
	}

	/* Logical Channel rate Request */
	if (msg->logicalChannelRateRequest_isSelected())
	{
	}

	/* Generic Request */
	if (msg->genericRequest_isSelected())
	{
	}


	/* Non standard */
	if (msg->nonStandard_isSelected ())
	{
	}

	printf("Unkown Message H245 Request!!\n");
	return 0;
}











/*	RESPONSE
*
*
*		
*/
size_t getH245MessageFromResponse (h324m_session *session,const MULTIMEDIA_SYSTEM_CONTROL::ResponseMessage* msg,unsigned char* buf)
{

	size_t size = 0;
	unsigned int n;
	/* TCS Ack */
	if(msg->terminalCapabilitySetAck_isSelected())
	{
		//printf("GOT TermCapAck\n");
		session->StateMachine |= STATE_TERMCAP;
		return 0;
	}

	/* TCS Reject */
	if(msg->terminalCapabilitySetReject_isSelected())
	{
		session->StateMachine &= !STATE_TERMCAP;
		//ep->localstep.step_TCS = 0;
	}

	/* 
	MSD Ack
	MSD is a 3-way Handshake, first peer A sends a MSD, peers B
	sends a MSDAck then Peer A sends a MSDAck
	*/ 
	if(msg->masterSlaveDeterminationAck_isSelected())
	{
		session->StateMachine |= STATE_MASTERSLAVE_DETERMINATION;
		if(msg->get_masterSlaveDeterminationAck().get_decision().master_isSelected()){
			printf("Received MasterSlaveAck: MASTER\n");
			return getLocalH245MasterSlaveDeterminationAck(session,buf,0);
		}
		else{
			printf("Received MasterSlaveAck: SLAVE\n");		

			return getLocalH245MasterSlaveDeterminationAck(session,buf,1);
		}
		return 0;
	}

	/* MSD Reject */
	if(msg->masterSlaveDeterminationReject_isSelected())
	{
	}
	/* OLC Ack */ 
	if(msg->openLogicalChannelAck_isSelected())
	{
		n = msg->get_openLogicalChannelAck().get_forwardLogicalChannelNumber().getValue();
		printf("WE RECEIVED OPEN LOGICAL CHANNEL ACK for %d \n",n);
		//ep->localState = MT;
		if(n == 1){
			session->StateMachine |= STATE_OLC_AUDIO;
			//	ep->localstep.step_OLCA = 1;
		}
		if(n == 2){
			session->StateMachine |= STATE_OLC_VIDEO;
			//	ep->localstep.step_OLCV = 1;
		}
		return 0;
	}

	/* OLC Reject */ 
	if(msg->openLogicalChannelReject_isSelected())
	{
	}

	/* Close LC Ack */ 
	if(msg->closeLogicalChannelAck_isSelected())
	{
	}

	/* Request Channel Close Ack */ 
	if(msg->requestChannelCloseAck_isSelected())
	{
	}

	/* MT Ack */
	if(msg->multiplexEntrySendAck_isSelected())
	{

		printf("WE RECEIVED MULTIPLEXENTRYSEND ACK for\n");
		session->StateMachine |= STATE_MULTIPLEX_ENTRY_SEND;
		//	ep->localstep.step_MES = 1;
		return 0;
	}

	/* MT Reject */
	if(msg->multiplexEntrySendReject_isSelected())
	{

		printf("WE RECEIVED MULTIPLEXENTRYSEND ACK for\n");

		return 0;

	}

	/* Request Multiplex Entry Ack */
	if(msg->requestMultiplexEntryAck_isSelected())
	{

		printf("WE RECEIVED MULTIPLEXENTRYSEND ACK for\n");

		return 0;
	}

	/* Request Multiplex Entry Reject */
	if(msg->requestMultiplexEntryReject_isSelected())
	{
	}

	/* Request Mode Ack */
	if(msg->requestModeAck_isSelected())
	{
	}

	/* Request Mode Reject */
	if(msg->requestModeReject_isSelected())
	{
	}

	/* Round Trip Delay Response */
	if(msg->roundTripDelayResponse_isSelected ())
	{
	}

	/* Maintenance Loop Ack */
	if(msg->maintenanceLoopAck_isSelected ())
	{
	}

	/* Maintenance Loop Reject */
	if(msg->maintenanceLoopReject_isSelected ())
	{
	}

	/* Communication Mode Response */
	if(msg->communicationModeResponse_isSelected ())
	{
	}

	/* Conference Response */
	if(msg->conferenceResponse_isSelected ())
	{
	}

	/* Multilink Response */
	if(msg->multilinkResponse_isSelected ())
	{
	}

	/* Logical Channel Rate Ack */
	if(msg->logicalChannelRateAcknowledge_isSelected())
	{
	}

	/* Logical Channel Rate Reject */
	if(msg->logicalChannelRateReject_isSelected())
	{
	}

	/* Generic Response */
	if (msg->genericResponse_isSelected ())
	{
	}

	/* Non standard */
	if (msg->nonStandard_isSelected ())
	{
	}
	fprintf (stderr, "RESPONSE message not recognized\n");
	return 0;
}













/*	INDICATION
*
*		
*/
size_t getH245MessageFromIndication (h324m_session  *session,  const MULTIMEDIA_SYSTEM_CONTROL::IndicationMessage* msg, unsigned char* buf)
{
	const char *dtmf;
	/* MSD Release */
	if(msg->masterSlaveDeterminationRelease_isSelected())
	{
	}

	/* Flow control */
	if(msg->flowControlIndication_isSelected())
	{
	}

	/* TCS Release */
	if(msg->terminalCapabilitySetRelease_isSelected())
	{
	}

	/* OpenLogicalChannel Confirm */
	if(msg->openLogicalChannelConfirm_isSelected())
	{
	}

	/* RequestChannelClose Release */
	if(msg->requestChannelCloseRelease_isSelected())
	{
	}

	/* MT Release */
	if(msg->multiplexEntrySendRelease_isSelected())
	{
	}

	/* Request Mode Release */
	if(msg->requestModeRelease_isSelected())
	{
	}

	/* Miscaellaneous */
	if(msg->miscellaneousIndication_isSelected())
	{
	}

	/* Jitter indication */
	if(msg->jitterIndication_isSelected())
	{
	}

	/* H223 Skew indication */
	if(msg->h223SkewIndication_isSelected())
	{
	}

	/* ATMVC indication */
	if(msg->newATMVCIndication_isSelected())
	{
	}

	/* User Input, DTMF */
	if(msg->userInput_isSelected())
	{
		if(msg->get_userInput().alphanumeric_isSelected()){
			dtmf = msg->get_userInput().get_alphanumeric().c_str();
			memcpy(session->DTMF,dtmf,strlen(dtmf));
		}
		return 0;
	}

	/* H.225.0 Maximum skew indication */
	if(msg->h2250MaximumSkewIndication_isSelected())
	{
	}

	/* mc Location indication */
	if(msg->mcLocationIndication_isSelected())
	{
	}

	/* conference indication */
	if(msg->conferenceIndication_isSelected ())
	{
	}

	/* conference indication */
	if(msg->conferenceIndication_isSelected ())
	{
	}

	/* Vendor Identification  */
	if (msg->vendorIdentification_isSelected ())
	{
		return 0;
	}

	/* Generic */
	if (msg->genericIndication_isSelected ())
	{
	}

	/* Non standard */
	if (msg->nonStandard_isSelected ())
	{
	}

	/* Function not understood */
	if (msg->functionNotUnderstood_isSelected ())
	{
	}

	/* Function not supported */
	if (msg->functionNotSupported_isSelected ())
	{
	}

	/* Multilink indication */
	if (msg->multilinkIndication_isSelected ())
	{
	}

	/* Logical Channel Rate Release */
	if (msg->logicalChannelRateRelease_isSelected ())
	{
	}

	/* Flow Control indication */
	if (msg->flowControlIndication_isSelected ())
	{
	}

	/* Mobile Multi link Reconfiguraiton indicaiton */
	if (msg->mobileMultilinkReconfigurationIndication_isSelected ())
	{
	}

	fprintf (stderr, "INDICATION message not recognized\n");
	return 0;
}






/*  COMMAND	
*
*		
*/
size_t getH245MessageFromCommand (h324m_session *session,  const MULTIMEDIA_SYSTEM_CONTROL::CommandMessage* msg,	   unsigned char* buf)
{
	/* genericCommand */
	if(msg->genericCommand_isSelected())
	{
		return 0;
	}

	/* mobileMultilinkReconfigurationCommand */
	if(msg->mobileMultilinkReconfigurationCommand_isSelected())
	{
		return 0;
	}

	/* newATMVCCommand */
	if(msg->newATMVCCommand_isSelected())
	{
		return 0;
	}

	/* h223MultiplexReconfiguration */
	if(msg->h223MultiplexReconfiguration_isSelected())
	{
		return 0;
	}

	/* conferenceCommand_isSelected */
	if(msg->conferenceCommand_isSelected())
	{
		return 0;
	}

	/* communicationModeCommand */
	if(msg->communicationModeCommand_isSelected())
	{
		return 0;
	}

	/* miscellaneousCommand */
	if(msg->miscellaneousCommand_isSelected())
	{
		fprintf (stderr, "GOT miscellaneousCommand\n");
		if(msg->get_miscellaneousCommand().get_type().videoFastUpdatePicture_isSelected()){
			return getLocalH245VideoFastUpdate(session,buf);
		}
		return 0;
	}

	/* endSessionCommand */
	if(msg->endSessionCommand_isSelected())
	{
		return 0;
	}

	/* flowControlCommand */
	if(msg->flowControlCommand_isSelected())
	{
		return 0;
	}

	/* encryptionCommand */
	if(msg->encryptionCommand_isSelected())
	{
		return 0;
	}

	/* sendTerminalCapabilitySet */
	if(msg->sendTerminalCapabilitySet_isSelected())
	{
		return 0;
	}

	/* maintenanceLoopOff */
	if(msg->maintenanceLoopOffCommand_isSelected())
	{
		return 0;
	}

	/* Non standard */
	if(msg->nonStandard_isSelected())
	{
		return 0;
	}

	fprintf (stderr, "COMMAND message not recognized\n");
	return 0;
}










extern "C"
size_t h245GetMessage(h324m_session *session, unsigned char* buf, unsigned int len, unsigned char *out)
{
	size_t size = 0;
	//	char *dbgbuf;

	std::stringstream mybuf;
	MULTIMEDIA_SYSTEM_CONTROL::MultimediaSystemControlMessage msg;
	ASN1::PERDecoder decoder((char*)buf , (char*)buf  + len);
	ASN1::AVNEncoder encoder(mybuf);


	while(msg.accept(decoder))
	{
		msg.accept(encoder);
		decoder.setPosition(decoder.getNextPosition());

		/* REQUEST messages */
		if (msg.request_isSelected())
		{
			size += getH245MessageFromRequest (session, &msg.get_request(), out + size);
		}
		/* RESPONSE messages */
		if (msg.response_isSelected())
		{
			size += getH245MessageFromResponse (session, &msg.get_response(), out  + size);
		}
		/* COMMAND messages */
		if (msg.command_isSelected())
		{
			size += getH245MessageFromCommand (session, &msg.get_command(), out  + size);
		}          
		/* INDICATION messages */
		if (msg.indication_isSelected())
		{
			size += getH245MessageFromIndication (session, &msg.get_indication(), out  + size);
		}          

	}

	return size;
}



