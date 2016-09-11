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
    int x, y, z;

    //init_platform();

    print("\r\n\r\n");
    print("Hello World\r\n");

    int size_x = 8, size_y = 5, size_z = 2;
    //int line_num = 3;
    //int via_num = 1;

    // テストデータ (マトリックス形式)
    char boardmat[MAX_LAYER][MAX_BOXES][MAX_BOXES];
    for (z = 0; z < MAX_LAYER; z++)
        for (y = 0; y < MAX_BOXES; y++)
            for (x = 0; x < MAX_BOXES; x++)
                boardmat[z][y][x] = -1;
    for (z = 0; z < size_z; z++)
        for (y = 0; y < size_y; y++)
            for (x = 0; x < size_x; x++)
                boardmat[z][y][x] = 0;
    // Line#1 (x=0, y=0, z=0) -- (x=7, y=4, z=0)
    boardmat[0][0][0] = 1; boardmat[0][4][7] = 1;
    // Line#2 (x=0, y=4, z=0) -- (x=7, y=0, z=1)
    boardmat[0][4][0] = 2; boardmat[1][0][7] = 2;
    // Line#3 (x=1, y=2, z=0) -- (x=2, y=2, z=0)
    boardmat[0][2][1] = 3; boardmat[0][2][2] = 3;
    // Via#a (x=4, y=2, z=0-1)
    boardmat[0][2][4] = 100; boardmat[1][2][4] = 100;

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
        int val;
        xil_printf("Start?\r\n");
        scanf("%d", &val);
        XNlsolver_Write_boardmat_V_Bytes (p_nlsolver, 0, (char *)boardmat, XNLSOLVER_AXI4LS_DEPTH_BOARDMAT_V);

        while ( !XNlsolver_IsIdle (p_nlsolver) );

        xil_printf("XNlsolver_Start\r\n");
        XNlsolver_Start (p_nlsolver);

        while ( !XNlsolver_IsDone (p_nlsolver) );

        xil_printf("XNlsolver_Reading\r\n");
        XNlsolver_Read_boardmat_V_Bytes (p_nlsolver, 0, (char *)boardmat, XNLSOLVER_AXI4LS_DEPTH_BOARDMAT_V);

        // 解表示
        xil_printf ("SOLUTION\r\n");
        xil_printf ("========\r\n");
        int x, y, z;
        for (z = 0; z < size_z; z++) {
            xil_printf ("LAYER %d\r\n", z + 1);
            for (y = 0; y < size_y; y++) {
                for (x = 0; x < size_x; x++) {
                    if (boardmat[z][y][x] < 100)
                        xil_printf (" %d", boardmat[z][y][x]);
                    else
                        xil_printf (" X");
                }
                xil_printf ("\r\n");
            }
        }
    }

    //cleanup_platform();
    return 0;
}
