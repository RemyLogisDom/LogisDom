
#ifndef API_DEF_H_
#define API_DEF_H_

typedef enum
{
        //! Order to enter in energy saving mode
        CO_WR_SLEEP = 1,
        //! Order to reset the device
        CO_WR_RESET = 2,
        //! Read the device (SW) version / (HW) version, chip ID etc.
        CO_RD_VERSION = 3,
        //! Read system log from device databank
        CO_RD_SYS_LOG = 4,
        //! Reset System log from device databank
        CO_WR_SYS_LOG = 5,
        //! Perform Flash BIST operation
        CO_WR_BIST = 6,
        //! Write ID range base number
        CO_WR_IDBASE = 7,
        //! Read ID range base number
        CO_RD_IDBASE = 8,
        //! Write Repeater Level off,1,2
        CO_WR_REPEATER = 9,
        //! Read Repeater Level off,1,2
        CO_RD_REPEATER = 10,
        //! Add filter to filter list
        CO_WR_FILTER_ADD = 11,
        //! Delete filter from filter list
        CO_WR_FILTER_DEL = 12,
        //! Delete filters
        CO_WR_FILTER_DEL_ALL = 13,
        //! Enable/Disable supplied filters
        CO_WR_FILTER_ENABLE = 14,
        //! Read supplied filters
        CO_RD_FILTER = 15,
        //! Waiting till end of maturity time before received radio telegrams will transmitted
        CO_WR_WAIT_MATURITY = 16,
        //! Enable/Disable transmitting additional subtelegram info
        CO_WR_SUBTEL = 17,
        //! Write x bytes of the Flash, XRAM, RAM0 ….
        CO_WR_MEM = 18,
        //! Read x bytes of the Flash, XRAM, RAM0 ….
        CO_RD_MEM = 19,
        //! Feedback about the used address and length of the config area and the Smart Ack Table
        CO_RD_MEM_ADDRESS = 20,
        //! Read security informations (level, keys)
        CO_RD_SECURITY = 21,
        //! Write security informations (level, keys)
        CO_WR_SECURITY = 22,
        //! Write mode, Advanced or Compatible mode
        CO_WR_MODE = 28,
        //! Read duty cycle limit
        CO_RD_DUTYCYCLE_LIMIT = 35
} COMMON_COMMAND_TYPE;

//! Filter kinds.
typedef enum
{
        //! Blocking radio filter
        BLOCK_RADIO = 0x00,
        //! Applying radio filter
        APPLY_RADIO = 0x80,
        //! Block filtered repeating
        BLOCK_REPEATED = 0x40,
        //! Apply filtered repeating
        APPLY_REPEATED = 0xC0
} FILTER_KIND;

//! Filter types.
typedef enum
{
        //! Source ID filter
        SOURCE_ID = 0,
        //! Choice filter
        CHOICE = 1,
        //! dBm filter
        DBM = 2,
        //! Destination ID filter
        DESTINATION_ID = 0
} FILTER_TYPE;

//! Filter operator.
typedef enum
{
        //! OR composition of filters
        OR = 0,
        //! AND composition of filters
        AND = 1
} FILTER_OPERATOR;

//! Read version response
typedef struct
{
        //! Application version
        uint8_t appVersion[4]; //Application versions
        //! Application description
        char appDescription[16]; //Applications description
        //! API version
        uint8_t apiVersion[4]; //API version
        //! Chip ID
        uint32_t chipID; //Unique ID
        //! Chip version (reserved for internal use)
        uint32_t chipVersion; //Reserved for internal use
} CO_RD_VERSION_RESPONSE;

//! Read base ID response
typedef struct
{
        //! Base ID
        uint32_t baseID; //Base ID
        //! Remaining write cycles for base ID
        uint8_t remainingWrites; //Remaining write cycles for Base ID
} CO_RD_IDBASE_RESPONSE;

//! Read repeater response
typedef struct
{
        //! Repeater enabled
        bool repEnable; //Repeater enable
        //! Repeater level
        uint8_t repLevel; //Repeater level
} CO_RD_REPEATER_RESPONSE;

//! Read filter response
typedef struct
{
        //! Filter type
        FILTER_TYPE filterType; //Filter type
        //! Filter value
        uint32_t filterValue; //Filter value
} CO_RD_FILTER_RESPONSE;

//! Common command enum
typedef enum
{
        //! Informs the backbone of a Smart Ack Client to not successful reclaim.
        SA_RECLAIM_NOT_SUCCESSFUL = 1,
        //! Used for SMACK to confirm/discard learn in/out
        SA_CONFIRM_LEARN = 2,
        //! Inform backbone about result of learn request
        SA_LEARN_ACK = 3,
        //! Inform backbone about about the readiness for operation
        SA_READY = 4,
        //! Informs backbone about duty cycle limit
        SA_DUTYCYCLE_LIMIT = 6
} SA_EVENT_TYPE;

//! Confirm code enum
typedef enum
{   //! Learn IN
        SA_LEARN_IN = 0x00,
        //! Discard Learn IN, EEP not accepted
        SA_EEP_NOT_ACCEPTED = 0x11,
        //! Discard Learn IN, PM has no place for further mailbox
        SA_PM_FULL = 0x12,
        //! Discard Learn IN, Controller has no place for new sensor
        SA_CONTROLLER_FULL = 0x13,
        //! Discard Learn IN, RSSI was not good enough
        SA_RSSI_NOT_GOOD = 0x14,
        //! Learn OUT
        SA_LEARN_OUT = 0x20,
        //! Function not supported
        SA_NOT_SUPPORTED = 0xFF
} SA_CONFIRM_CODE;

//! Reset cause according ESP 3
typedef enum
{
        //! Voltage supply drop or indicates that VDD > VON
        EC_VOLTAGE_DROP = 0x00,
        //! Reset caused by usage of the reset pin
        EC_RESET_PIN = 0x01,
        //! Watchdog timer counter reached the time period
        EC_WATCHDOG_TIMER = 0x02,
        //! Flywheel timer counter reached the time period
        EC_FLYWHEEL_TIMER = 0x03,
        //! Parity error
        EC_PARITY_ERROR = 0x04,
        //! HW Parity error in internal or external memory
        EC_PARITY_MEM_ERROR = 0x05,
        //! CPU memory request does not correspond to valid memory location
        EC_MEM_ERRPR = 0x06,
        //! Wake-up pint 0 activated
        EC_WAKE_PIN_0 = 0x07,
        //! Wake-up pint 1 activated
        EC_WAKE_PIN_1 = 0x08,
        //! Unknown reset source
        EC_UNKNOWN = 0x09
} EC_RESET_CAUSE;

//! Types of mode
typedef enum
{
        //! Compatible mode (default)
        MODE_COMPATIBLE = 0x00,
        //! Advanced mode
        MODE_ADVANCED = 0x01
} WR_MODE_CODE;

//! Confirm Learn response
typedef struct
{
        //! Already post master	0b xxxx 1xxx/Place for mailbox	0b xxxx x1xx/Good RSSI	0b xxxx xx1x/Local	0b xxxx xxx1
        uint8_t priPostmaster;
        //! Manufacturer ID
        uint32_t manufacturerID;
        //! Code of used EEP profile
        uint8_t eep[3];
        //! Signal strength; Send case: FF/Receive case: actual RSSI
        uint8_t rssi;
        //! Device ID of the Post master candidate
        uint32_t postmasterID;
        //! This sensor would be Learn IN
        uint32_t smartAckID;
        //! Numbers of repeater hop
        uint8_t hopCount;
} SA_CONFIRM_LEARN_REQUEST;

//! Learn SmartAck response
typedef struct
{
        //! Response time for Smart Ack Client in ms in which the controller can prepare the data and send it to the postmaster. Only actual if learn return code is Learn IN
        uint16_t responseTime;
        //! Confirmation Code
        SA_CONFIRM_CODE confirmCode;
} SA_LEARN_ACK_RESPONSE;

//! Ready response
typedef struct
{
        //! Cause of reset
        EC_RESET_CAUSE resetCause;
} CO_READY_RESPONSE;

//! SmartAck command enum
typedef enum
{
        //! Set/Reset Smart Ack learn mode
        SA_WR_LEARNMODE = 1,
        //! Get Smart Ack learn mode state
        SA_RD_LEARNMODE = 2,
        //! Used for Smart Ack to add or delete a mailbox of a client
        SA_WR_LEARNCONFIRM = 3,
        //! Send Smart Ack Learn request (Client)
        SA_WR_CLIENTLEARNRQ = 4,
        //! Send reset command to every SA sensor
        SA_WR_RESET = 5,
        //! Get Smart Ack learned sensors / mailboxes
        SA_RD_LEARNEDCLIENTS = 6,
        //! Set number of reclaim attempts
        SA_WR_RECLAIMS = 7,
        //! Activate/Deactivate Post master functionality
        SA_WR_POSTMASTER = 8
} SA_COMMAND_TYPE;

//! SmartAck confirm code enum send to gateway in serial command
typedef enum
{
        //! Learn IN
        SA_CC_LEARNIN = 0X00,
        //! Learn OUT
        SA_CC_LEARNOUT = 0X20
} SMARTACK_CONFIRM_CODE;

//! SmartAck extended learnmode enum
typedef enum
{
        //! Simple Learnmode
        SA_EL_SIMPLE = 0X00,
        //! Advanced Learnmode
        SA_EL_ADVANCE = 0X01,
        //! Advanced Learnmode select Rep.
        SA_EL_ADVANCE_REP = 0X02
} SA_EXTENDED_LEARNMODE;

//! SmartAck read learnmode response structure
typedef struct
{
        //! Learnmode not active = 0/Learnmode active = 1
        bool enabled;
        //! Simple Learnmode = 0/Advance Learnmode = 1/Advance Learnmode select Rep. = 2
        SA_EXTENDED_LEARNMODE extended;
} SA_RD_LEARNMODE_RESPONSE;

//! SmartAck learned clients response structure
typedef struct
{
        //! Device ID of the Smart Ack Client
        uint32_t clientID;
        //! Postmaster ID dedicated Smart Ack Client
        uint32_t postmasterID;
        //! Internal counter of Post master (0x00 ... 0x0E)
        uint8_t mailboxIndex;

} SA_RD_LEARNEDCLIENTS_RESPONSE;

//! Duty cycle limit enum
typedef enum
{
        //! Duty cycle limit released, possible to send telegrams
        CO_DUTYCYCLE_LIMIT_UNREACHED = 0x00,
        //! Duty cycle limit reached, no more telegrams will be sent
        CO_DUTYCYCLE_LIMIT_REACHED = 0x01
} CO_DUTYCYCLE_LIMIT;

//! Read actual duty cycle limit structure
typedef struct
{
        //! Available duty cycle
        uint8_t availableCycle;
        //! Number of duty cycle slots
        uint8_t numOfSlots;
        //! Period of one slot in seconds
        uint16_t slotPeriod;
        //! Time left in actual slot in seconds
        uint16_t slotPeriodLeft;
        //! Load available when period ends
        uint8_t loadAfterCycle;
} CO_RD_DUTYCYCLE_RESPONSE;



typedef enum
{
	//! Return values are the same as for the ESP3 command response
	//! <b>0x00</b> - Action performed. No problem detected
	EO_OK = 0x00, //!EO_OK ... command is understood and triggered
	//! <b>0x01</b> - Generic Error
	EO_ERROR = 0x01, //!There is an error occurred
	//! <b>0x02</b> - The functionality is not supported by that implementation
	NOT_SUPPORTED = 0x02, //!The functionality is not supported by that implementation
	//! <b>0x03</b> - There was a wrong function parameter
	WRONG_PARAM = 0x03, //!Wrong function parameter
	//! <b>0x04</b> - Operation denied
	OPERATION_DENIED = 0x04, //!Example: memory access denied (code-protected)
	//! <b>0x05</b> - Lock set
	LOCK_SET = 0x05, //!Lock set
	//! <b>0x06</b> - Buffer too small
	BUFFER_TOO_SMALL = 0x06, //!Buffer too small
	//! <b>0x07</b> - No free buffer
	NO_FREE_BUFFER = 0x06, //!No free buffer
	//! EO-Link specific return codes
	//! <b>0x55</b> - Action couldn't be carried out within a certain time.
	TIME_OUT = 0x50, //!< TIME_OUT
	//! <b>0x51</b> - No byte or telegram received
	NO_RX = 0x51, //!< NO_RX
	//! <b>0x52</b> - The UART receive is ongoing
	ONGOING_RX = 0x52, //!< ONGOING_RX
	//! <b>0x53</b> - A new byte or telegram received
	NEW_RX = 0x53, //!< NEW_RX
	//! <b>0x54</b> - Buffer full, no space in Tx or Rx buffer
	BUFF_FULL = 0x54, //!< BUFF_FULL
	//!	<b>0x55</b> - Generic out of range return code e.g. address is out of range or the buffer is too small
	OUT_OF_RANGE = 0x55, //!< OUT_OF_RANGE
	//! <b>0x56</b> - Error Opening Port
	PORT_ERROR = 0x56, //!< PORT_ERROR
	//! <b>0x57</b> - User command
	USER_CMD = 0x57,//!Return codes greater than 0x80 are used for commands with special return information, not commonly useable.
	//! <b>0x81</b> Received message is not a recom message
	NOT_RECOM = 0x81,
	//! <b>0x90</b> Invalid Packet
	INVALID_PACKET=0x90,
	//! <b>0xFF</b> - This function is not implemented
	NOT_IMPLEMENTED = 0xFF  //!< NOT_IMPLEMENTED
} eoReturn;

//! Packet type (ESP3)
typedef enum
{
	PACKET_RESERVED = 0x00,	//!Reserved
	PACKET_RADIO = 0x01,	//!Radio telegram
	PACKET_RESPONSE = 0x02,	//!Response to any packet
	PACKET_RADIO_SUB_TEL = 0x03,//!Radio subtelegram (EnOcean internal function�)
	PACKET_EVENT = 0x04,	//!Event message
	PACKET_COMMON_COMMAND = 0x05,	//!Common command
	PACKET_SMART_ACK_COMMAND = 0x06,	//!Smart Ack command
	PACKET_REMOTE_MAN_COMMAND = 0x07,	//!Remote management command
	PACKET_PRODUCTION_COMMAND = 0x08,	//!Production command
	PACKET_RADIO_MESSAGE = 0x09,	///!Radio message (chained radio telegrams)
	PACKET_RADIO_ADVANCED  = 0x0A, //!Advanced protocol radio telegram
	PACKET_RADIO_SET = 0x0E,	//!set radio stuff&answear
	PACKET_DEBUG = 0x0F	//!debug message
} PACKET_TYPE;

//! Telegram choice codes applies to radio telegram only
typedef enum
{
	//! RPS telegram
	RADIO_CHOICE_RPS = 0xF6,
	//! 1BS telegram
	RADIO_CHOICE_1BS = 0xD5,
	//! 4BS telegram
	RADIO_CHOICE_4BS = 0xA5,
	//! HRC telegram
	RADIO_CHOICE_HRC = 0xA3,
	//! SYS telegram
	RADIO_CHOICE_SYS = 0xA4,
	//! SYS_EX telegram
	RADIO_CHOICE_SYS_EX = 0xC5,
	//! Smart Ack Learn Request telegram
	RADIO_CHOICE_SM_LRN_REQ = 0xC6,
	//! Smart Ack Learn Answer telegram
	RADIO_CHOICE_SM_LRN_ANS = 0xC7,
	//! Smart Ack Reclaim telegram
	RADIO_CHOICE_RECLAIM = 0xA7,
	//! Smart Request telegram
	RADIO_CHOICE_SIGNAL = 0xD0,
	//! Encapsulated addressable telegram
	RADIO_CHOICE_ADT = 0xA6,
	//! Variable Length Data
	RADIO_CHOICE_VLD = 0xD2,
	//! Universal Teach In EEP based
	RADIO_CHOICE_UTE = 0xD4,
	//! Manufacturer Specific Communication
	RADIO_CHOICE_MSC = 0xD1,
	//! Single Teach-In Request
	RADIO_CHOICE_GP_SINGLE_TREQ = 0xB0,
	//! Chained Teach-In Request
	RADIO_CHOICE_GP_CHAINED_TREQ = 0xB1,
	//! Single Teach-In Response
	RADIO_CHOICE_GP_SINGLE_TRES = 0xB2,
	//! Chained Teach-In Response
	RADIO_CHOICE_GP_CHAINED_TRES = 0xB3,
	//! Single Data
	RADIO_CHOICE_GP_SINGLE_DATA = 0xB4,
	//! Chained Data
	RADIO_CHOICE_GP_CHAINED_DATA = 0xB5,
	//! Selective data
	RADIO_CHOICE_GP_SELECT_DATA = 0xB6,
	//! Secure telegram
	RADIO_CHOICE_SEC = 0x30,
	//! Secure telegram	with choice encapsulation
	RADIO_CHOICE_SEC_ENCAPS = 0x31,
	//! Non-secure telegram
	RADIO_CHOICE_NON_SEC = 0x32,
	//! Secure teach-in telegram
	RADIO_CHOICE_SEC_TI = 0x35

} CHOICE_TYPE;
//! RORGS
typedef enum
{
	//! RPS telegram
	RORG_RPS = 0xF6,
	//! 1BS telegram
	RORG_1BS = 0xD5,
	//! 4BS telegram
	RORG_4BS = 0xA5,
	//! HRC telegram
	RORG_HRC = 0xA3,
	//! SYS telegram
	RORG_SYS = 0xA4,
	//! SYS_EX telegram
	RORG_SYS_EX = 0xC5,
	//! Smart Ack Learn Request telegram
	RORG_SM_LRN_REQ = 0xC6,
	//! Smart Ack Learn Answer telegram
	RORG_SM_LRN_ANS = 0xC7,
	//! Smart Ack Reclaim telegram
	RORG_RECLAIM = 0xA7,
	//! Smart Request telegram
	RORG_SIGNAL = 0xD0,
	//! Encapsulated addressable telegram
	RORG_ADT = 0xA6,
	//! Teach-in request
	GP_TI = 0xB0,
	//!Teach-in response
	GP_TR = 0xB1,
	//! Complete Data
	GP_CD = 0xB2,
	//!Selective data
	GP_SD = 0xB3,
	//! Variable Length Data
	RORG_VLD = 0xD2,
	//! Universal Teach In EEP based
	RORG_UTE = 0xD4,
	//! Manufacturer Specific Communication
	RORG_MSC = 0xD1,
	//! Chained data message
	RORG_CDM = 0x40,
	//! Secure telegram	without choice encapsulation
	RORG_SEC = 0x30,
	//! Secure telegram	with choice encapsulation
	RORG_SEC_ENCAPS = 0x31,
	//! Secure telegram decrypted
	RORG_SECD = 0x32,
	//! Secure teach-in telegram
	RORG_SEC_TI = 0x35
} RORG_TYPE;

//!UTE Response codes.
typedef enum
{
        //! <b>0</b> - Request not accepted because of general reason
        GENERAL_REASON		 = 0,         //!< GENERAL_REASON
        //! <b>1</b> - Request accepted, teach-in successful
        TEACH_IN_ACCEPTED	 = 1,         //!< TEACH_IN_ACCEPTED
        //! <b>2</b> - Request accepted, deletion of teach-in successful
        TEACH_OUT_ACCEPTED 	= 2,         //!< TEACH_OUT_ACCEPTED
        //! <b>3</b> - Request not accepted, EEP not supported
        EEP_NOT_SUPPORTED	 = 3         //!< EEP_NOT_SUPPORTED
} UTE_RESPONSE;

//!UTE Direction response codes.
typedef enum
{
        //! <b>0</b> - Unidirectional
        UTE_DIRECTION_UNIDIRECTIONAL		 = 0,         //!< UTE_DIRECTION_UNIDIRECTIONAL
        //! <b>1</b> - Bidirectional
        UTE_DIRECTION_BIDIRECTIONAL			 = 1         //!< UTE_DIRECTION_BIDIRECTIONAL
} UTE_DIRECTION;

/**
 * \typedef GP_RESPONSE_RESULT
 * \brief Result after a Teach IN
 */
typedef enum
{
        RESP_REJECTED_GENERALLY = 0,
        RESP_TEACHIN,
        RESP_TEACHOUT,
        RESP_REJECTED_CHANNELS
} GP_RESPONSE_RESULT;


//!Destination broadcast ID
#define  BROADCAST_ID       0xFFFFFFFF

#endif /* API_DEF_H_ */
