#ifndef _H245_TEMPLATES
#define _H245_TEMPLATES

static char template_TerminalCapabilitySetB[] = 
"request : terminalCapabilitySet : {\n\
  sequenceNumber 1,\n\
  protocolIdentifier { 0 0 8 245 0 6 },\n\
  multiplexCapability h223Capability : {\n\
    transportWithI-frames FALSE,\n\
    videoWithAL1 FALSE,\n\
    videoWithAL2 TRUE,\n\
    videoWithAL3 TRUE,\n\
    audioWithAL1 FALSE,\n\
    audioWithAL2 TRUE,\n\
    audioWithAL3 FALSE,\n\
    dataWithAL1 FALSE,\n\
    dataWithAL2 FALSE,\n\
    dataWithAL3 FALSE,\n\
    maximumAl2SDUSize 4096,\n\
    maximumAl3SDUSize 4096,\n\
    maximumDelayJitter 10,\n\
    h223MultiplexTableCapability basic : NULL,\n\
    maxMUXPDUSizeCapability TRUE,\n\
    nsrpSupport TRUE,\n\
    mobileOperationTransmitCapability {\n\
      modeChangeCapability FALSE,\n\
      h223AnnexA FALSE,\n\
      h223AnnexADoubleFlag FALSE,\n\
      h223AnnexB TRUE,\n\
      h223AnnexBwithHeader FALSE\n\
    }\n\
  },\n\
  capabilityTable {\n\
    {\n\
      capabilityTableEntryNumber 1,\n\
      capability receiveAudioCapability : genericAudioCapability : {\n\
        capabilityIdentifier standard : { 0 0 8 245 1 1 1 },\n\
        maxBitRate 122,\n\
        collapsing {\n\
          {\n\
            parameterIdentifier standard : 0,\n\
            parameterValue unsignedMin : 1\n\
          }\n\
        }\n\
      }\n\
    },\n\
    {\n\
      capabilityTableEntryNumber 2,\n\
      capability receiveVideoCapability : genericVideoCapability : {\n\
        capabilityIdentifier standard : { 0 0 8 245 1 0 0 },\n\
        maxBitRate 521,\n\
        nonCollapsing {\n\
          {\n\
            parameterIdentifier standard : 0,\n\
            parameterValue unsignedMax : 8\n\
          }\n\
        }\n\
      }\n\
    },\n\
    {\n\
      capabilityTableEntryNumber 3,\n\
      capability receiveVideoCapability : h263VideoCapability : {\n\
        qcifMPI 2,\n\
        maxBitRate 521,\n\
        unrestrictedVector FALSE,\n\
        arithmeticCoding FALSE,\n\
        advancedPrediction FALSE,\n\
        pbFrames FALSE,\n\
        temporalSpatialTradeOffCapability FALSE,\n\
        errorCompensation FALSE\n\
      }\n\
    },\n\
    {\n\
      capabilityTableEntryNumber 4,\n\
      capability receiveUserInputCapability : dtmf : NULL\n\
    },\n\
    {\n\
      capabilityTableEntryNumber 5,\n\
      capability receiveUserInputCapability : iA5String : NULL\n\
    },\n\
    {\n\
      capabilityTableEntryNumber 6,\n\
      capability receiveUserInputCapability : basicString : NULL\n\
    }\n\
  },\n\
  capabilityDescriptors {\n\
    {\n\
      capabilityDescriptorNumber 0,\n\
      simultaneousCapabilities {\n\
        {\n\
          1\n\
        },\n\
        {\n\
          2,\n\
          3\n\
        },\n\
        {\n\
          4,\n\
          5,\n\
          6\n\
        }\n\
      }\n\
    }\n\
  }\n\
}\n";


static char template_TerminalCapabilitySet[] = 
"request : terminalCapabilitySet : {\n\
  sequenceNumber %d,\n\
  protocolIdentifier { 0 0 8 245 0 6 },\n\
  multiplexCapability h223Capability : {\n\
    transportWithI-frames FALSE,\n\
    videoWithAL1 FALSE,\n\
    videoWithAL2 TRUE,\n\
    videoWithAL3 FALSE,\n\
    audioWithAL1 FALSE,\n\
    audioWithAL2 TRUE,\n\
    audioWithAL3 FALSE,\n\
    dataWithAL1 FALSE,\n\
    dataWithAL2 FALSE,\n\
    dataWithAL3 FALSE,\n\
    maximumAl2SDUSize 1024,\n\
    maximumAl3SDUSize 1024,\n\
    maximumDelayJitter 10,\n\
    h223MultiplexTableCapability basic : NULL,\n\
    maxMUXPDUSizeCapability FALSE,\n\
    nsrpSupport TRUE,\n\
    mobileOperationTransmitCapability {\n\
      modeChangeCapability FALSE,\n\
      h223AnnexA FALSE,\n\
      h223AnnexADoubleFlag FALSE,\n\
      h223AnnexB TRUE,\n\
      h223AnnexBwithHeader FALSE\n\
    }\n\
  },\n\
   capabilityTable {\n\
    {\n\
      capabilityTableEntryNumber 1,\n\
      capability receiveAndTransmitAudioCapability : genericAudioCapability : {\n\
        capabilityIdentifier standard : { 0 0 8 245 1 1 1 },\n\
        maxBitRate 122,\n\
        collapsing {\n\
          {\n\
            parameterIdentifier standard : 0,\n\
            parameterValue unsignedMin : 1\n\
          }\n\
        }\n\
      }\n\
    },\n\
    {\n\
      capabilityTableEntryNumber 3,\n\
      capability receiveAndTransmitVideoCapability : h263VideoCapability : {\n\
        sqcifMPI 2,\n\
        qcifMPI 2,\n\
        maxBitRate 600,\n\
        unrestrictedVector FALSE,\n\
        arithmeticCoding FALSE,\n\
        advancedPrediction FALSE,\n\
        pbFrames FALSE,\n\
        temporalSpatialTradeOffCapability TRUE,\n\
        errorCompensation FALSE\n\
      }\n\
    },\n\
    {\n\
      capabilityTableEntryNumber 7,\n\
      capability receiveUserInputCapability : iA5String : NULL\n\
    },\n\
    {\n\
      capabilityTableEntryNumber 9,\n\
      capability receiveUserInputCapability : dtmf : NULL\n\
    }\n\
  },\n\
  capabilityDescriptors {\n\
    {\n\
      capabilityDescriptorNumber 1,\n\
      simultaneousCapabilities {\n\
        {\n\
          1\n\
        },\n\
        {\n\
          3\n\
        },\n\
		{\n\
		  7,\n\
		  9\n\
		}\n\
      }\n\
    }\n\
  }\n\
}\n";




static char template_TerminalCapabilitySetAck[] = 
"response : terminalCapabilitySetAck : {\n\
  sequenceNumber %d\n\
}\n";





static char template_MasterSlaveDetermination[] = 
"request : masterSlaveDetermination : {\n\
  terminalType %d,\n\
  statusDeterminationNumber %d\n\
}\n";


static char template_MasterSlaveDeterminationAckSlave[] = 
"response : masterSlaveDeterminationAck : {\n\
  decision slave : NULL\n\
}\n";


static char template_MasterSlaveDeterminationAckMaster[] = 
"response : masterSlaveDeterminationAck : {\n\
  decision master : NULL\n\
}\n";


static char template_OpenLogicalChannelAMR[] = 
"request : openLogicalChannel : {\n\
  forwardLogicalChannelNumber 1,\n\
  forwardLogicalChannelParameters {\n\
    dataType audioData : genericAudioCapability : {\n\
      capabilityIdentifier standard : { 0 0 8 245 1 1 1 },\n\
      maxBitRate 122,\n\
      collapsing {\n\
        {\n\
          parameterIdentifier standard : 0,\n\
          parameterValue unsignedMin : 1\n\
        }\n\
      }\n\
    },\n\
    multiplexParameters h223LogicalChannelParameters : {\n\
      adaptationLayerType al2WithSequenceNumbers : NULL,\n\
      segmentableFlag FALSE\n\
    }\n\
  }\n\
}\n";




static char template_OpenLogicalChannelH263[] = 
"request : openLogicalChannel : {\n\
  forwardLogicalChannelNumber 2,\n\
  forwardLogicalChannelParameters {\n\
    dataType videoData : h263VideoCapability : {\n\
      qcifMPI 2,\n\
      maxBitRate 500,\n\
      unrestrictedVector FALSE,\n\
      arithmeticCoding FALSE,\n\
      advancedPrediction FALSE,\n\
      pbFrames FALSE,\n\
      temporalSpatialTradeOffCapability TRUE,\n\
      errorCompensation FALSE\n\
    },\n\
    multiplexParameters h223LogicalChannelParameters : {\n\
      adaptationLayerType al2WithSequenceNumbers : NULL,\n\
      segmentableFlag TRUE\n\
    }\n\
  }\n\
}\n";



static char template_MultiplexEntrySend[] = 
"request : multiplexEntrySend : {\n\
  sequenceNumber 0,\n\
  multiplexEntryDescriptors {\n\
    {\n\
      multiplexTableEntryNumber 1,\n\
      elementList {\n\
        {\n\
          type logicalChannelNumber : 1,\n\
          repeatCount untilClosingFlag : NULL\n\
        }\n\
      }\n\
    },\n\
    {\n\
      multiplexTableEntryNumber 2,\n\
      elementList {\n\
        {\n\
          type logicalChannelNumber : 2,\n\
          repeatCount untilClosingFlag : NULL\n\
        }\n\
      }\n\
    }\n\
  }\n\
}\n";





static char template_VideoFastUpdate[] = 
"command : miscellaneousCommand : {\n\
  logicalChannelNumber 2,\n\
  type videoFastUpdatePicture : NULL\n\
}\n";


static char template_OpenLogicalChannelAck[] = 
"response : openLogicalChannelAck : {\n\
  forwardLogicalChannelNumber %d\n\
}\n";



static char template_CloseLogicalChannelAck[] = 
"response : closeLogicalChannelAck : {\n\
  forwardLogicalChannelNumber %d\n\
}\n";


static char template_roundTripleDelayResponse[] = 
"response : roundTripDelayResponse : {\n\
  sequenceNumber %d\n\
}\n";

static char template_roundTripleDelayRequest[] = 
"request : roundTripDelayRequest : {\n\
	sequenceNumber %d\n\
}\n";


static char template_multiplexEntrySendAck[] =
"response : multiplexEntrySendAck : {\n\
  sequenceNumber %d,\n\
  multiplexTableEntryNumber {\n\
	1,\n\
	2,\n\
	3,\n\
	4,\n\
	5,\n\
	6,\n\
	7,\n\
	8,\n\
	9,\n\
	10,\n\
	11,\n\
	12,\n\
	13,\n\
	14,\n\
	15\n\
  }\n\
}\n";


#endif // _H245_TEMPLATES


