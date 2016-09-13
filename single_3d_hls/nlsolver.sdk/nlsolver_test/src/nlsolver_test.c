/*
 * nlsolver_test.c
 *
 *  Created on: 2016/09/10
 *      Author: kotaro.terada
 */

#include <stdio.h>
//#include "platform.h"
#include "xil_printf.h"

#include "xnlsolver.h"

#define MAX_BOXES 40
#define MAX_LAYER 8

int main()
{
    //init_platform();

    print("\r\n\r\n");
    print("Hello World\r\n");

    XNlsolver nlsolver;
    XNlsolver *p_nlsolver = &nlsolver;
    XNlsolver_Config *p_nlsolver_config = NULL;

    // Look Up the device configuratoin
    p_nlsolver_config = XNlsolver_LookupConfig (XPAR_NLSOLVER_0_DEVICE_ID);
    if (!p_nlsolver_config) {
        fprintf (stderr, "XNlsolver Config Lookup failed.\r\n");
        return -1;
    }

    // Initialize the Device
    int XNlsolver_status = XNlsolver_CfgInitialize (p_nlsolver, p_nlsolver_config);
    if (XNlsolver_status != XST_SUCCESS) {
        fprintf (stderr, "XNlsolver Configuration failed.\r\n");
        return -1;
    }

    while (1) {
        char str[12800];

        xil_printf("Problem?\r\n");
        scanf("%s", str);
        xil_printf("%s\r\n", str);
        int size_x = (str[1] - '0') * 10 + (str[2] - '0');
        int size_y = (str[4] - '0') * 10 + (str[5] - '0');
        int size_z = (str[7] - '0');

        XNlsolver_Write_boardstr_Bytes (p_nlsolver, 0, str, XNLSOLVER_AXI4LS_DEPTH_BOARDSTR);

        while ( !XNlsolver_IsIdle (p_nlsolver) );

        xil_printf("XNlsolver_Start\r\n");
        XNlsolver_Start (p_nlsolver);

        while ( !XNlsolver_IsDone (p_nlsolver) );
        xil_printf("Done!\r\n");

        //xil_printf("XNlsolver_Read_boardstr_Bytes\r\n");
        XNlsolver_Read_boardstr_Bytes (p_nlsolver, 0, str, XNLSOLVER_AXI4LS_DEPTH_BOARDSTR);
        int status = XNlsolver_Get_status_V (p_nlsolver);

        if (status == 0)
            xil_printf("Passed!\r\n");
        else
            xil_printf("Failed!\r\n");
        xil_printf("status = %d\r\n\r\n", status);

        // 解表示
        xil_printf ("SOLUTION\r\n");
        xil_printf ("========\r\n");
        int x, y, z, i = 0;
        xil_printf("SIZE %dX%dX%d\r\n", size_x, size_y, size_z);
        for (z = 0; z < size_z; z++) {
            xil_printf ("LAYER %d\r\n", z + 1);
            for (y = 0; y < size_y; y++) {
                for (x = 0; x < size_x; x++) {
                    if (x != 0)
                        xil_printf (",");
                    xil_printf ("%02d", (int)(str[i]));
                    i++;
                }
                xil_printf ("\r\n");
            }
        }
        xil_printf ("========\r\n\r\n");
    }

    //cleanup_platform();
    return 0;
}
