/*
 * MRF24WG scan functions.
 *
 * Functions to initiate a scan and retrieve scan results.
 */
#include <stdio.h>
#include "wf_universal_driver.h"
#include "wf_global_includes.h"

/*
 * Commands the MRF24W to start a scan operation.  This will generate the
 * WF_EVENT_SCAN_RESULTS_READY event.
 *
 * Directs the MRF24W to initiate a scan operation utilizing the input
 * Connection Profile ID.  The Host Application will be notified that the scan
 * results are ready when it receives the WF_EVENT_SCAN_RESULTS_READY event.
 * The eventInfo field for this event will contain the number of scan results.
 * Once the scan results are ready they can be retrieved with
 * WF_ScanResultGet().
 *
 * Scan results are retained on the MRF24W until:
 *  1.  Calling WF_Scan() again (after scan results returned from previous
 *      call).
 *  2.  MRF24W reset.
 *
 * Parameters:
 *  mode - WF_SCAN_FILTERED or WF_SCAN_ALL
 *
 *    WF_SCAN_FILTERED:
 *          * If SSID defined only scan results with that SSID are retained
 *          * If SSID not defined all scanned SSID?s will be retained
 *          * Only scan results from Infrastructure or AdHoc networks are
 *             retained, depending on the configured network type
 *          * The only channels scanned are those set in WF_SetChannelList()
 *
 *    WF_SCAN_ALL:
 *          * Can be called after WF_Init() successfully completes (see WF_INIT_SUCCESSFUL eventData)
 *          * All scan results are retained (both Infrastructure and Ad Hoc networks).
 *          * All channels within the MRF24W's regional domain will be scanned.
 */
void WF_Scan(uint8_t scanMode)
{
    uint8_t connectionState;
    uint8_t   hdr[4];

#if defined(WF_ERROR_CHECKING)
    uint32_t errorCode = UdScan(scanMode);
    if (errorCode != UD_SUCCESS) {
        EventEnqueue(WF_EVENT_ERROR, errorCode);
        return;
    }
#endif

    // Can only scan when connected or idle
    WF_ConnectionStateGet(&connectionState);
    if (connectionState == WF_CSTATE_CONNECTION_IN_PROGRESS ||
        connectionState == WF_CSTATE_RECONNECTION_IN_PROGRESS) {
        EventEnqueue(WF_EVENT_ERROR, UD_ERROR_SCAN_NOT_ALLOWED);
        return;
    }

    hdr[0] = WF_MGMT_REQUEST_TYPE;
    hdr[1] = WF_SCAN_START_SUBTYPE;
    hdr[2] = (scanMode == WF_SCAN_FILTERED) ? GetCpid() : 0xff;
    hdr[3] = 0; /* not used */
    SendMgmtMsg(hdr, sizeof(hdr), NULL, 0);

    /* wait for mgmt response, free it after it comes in (no data needed) */
    WaitForMgmtResponse(WF_SCAN_START_SUBTYPE, FREE_MGMT_BUFFER);
}

/*
 * Read scan results back from MRF24W.
 * The RSSI value in the scan result will range from 43 to 106
 *
 * After a scan has completed this function is used to read one or more of the
 * scan results from the MRF24W.  The scan results will be written
 * contiguously starting at p_scanResults (see tWFScanResult structure for
 * format of scan result).
 *
 * MACInit must be called first.  WF_EVENT_SCAN_RESULTS_READY event must have
 * already occurrerd.
 *
 * Parameters:
 *  listIndex    - Index (0-based list) of the scan entry to retrieve.
 *  p_scanResult - Pointer to location to store the scan result structure
 */
void WF_ScanResultGet(uint8_t listIndex, t_scanResult *p_scanResult)
{
    uint8_t   hdr[4];
    /* char rssiChan[48]; */ /* reference for how to retrieve RSSI */

    hdr[0] = WF_MGMT_REQUEST_TYPE;
    hdr[1] = WF_SCAN_GET_RESULTS_SUBTYPE;
    hdr[2] = listIndex;        /* scan result index to read from */
    hdr[3] = 1;                /* number of results to read */
    SendMgmtMsg(hdr, sizeof(hdr), NULL, 0);

    /* index 4 contains number of scan results returned, index 5 is first byte of first scan result */
    WaitForMgmtResponseAndReadData(
        WF_SCAN_GET_RESULTS_SUBTYPE,    /* expected subtype */
        sizeof(t_scanResult),           /* num data bytes to read */
        5,                              /* starting at this index */
        (uint8_t *)p_scanResult);       /* write the response data here */

    /* fix up endianness for the two 16-bit values in the scan results */
    p_scanResult->beaconPeriod = htons(p_scanResult->beaconPeriod);
    p_scanResult->atimWindow   = htons(p_scanResult->atimWindow);
}
