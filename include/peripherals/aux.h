#pragma once

#include "common.h"

#include "peripherals/base.h"

struct AuxRegs {
    reg32 irq_status;
    reg32 enables;
    reg32 reserved[14];
    reg32 mu_io;
    reg32 mu_ier;
    reg32 mu_iir;
    reg32 mu_lcr;
    reg32 mu_mcr;
    reg32 mu_lsr;
    reg32 mu_msr;
    reg32 mu_scratch;
    reg32 mu_control;
    reg32 mu_status;
    reg32 mu_baud_rate;
};
struct UartRegs{
    reg32 u_dr;      //data reg
    reg32 reserved;
    reg32 u_fr;      //flag reg
    reg32 u_ilpr;    // not in use
    reg32 u_ibrd;    //integer baud rate divisor
    reg32 u_fbrd;    //Fractional Baud rate divisor
    reg32 u_lcrh;    //line control register
    reg32 u_cr;      //control register
    reg32 u_ifls;    // interrupt fifo level select reg    
    reg32 u_imsc;    //interrupt mask set clear reg
    reg32 u_ris;     //raw interrupt status reg
    reg32 u_mis;     //masked interrupt status reg
    reg32 u_icr;     //interrupt clear register
    reg32 u_dmacr;   //dma controller regs (not suppirted in rpi3)
    reg32 u_itcr;    //test control reg
    reg32 u_itip;    //integraton test input reg
    reg32 u_itop;    //integration test output reg
    reg32 u_tdr;     //test data reg
};
#define REGS_AUX ((struct AuxRegs *)(PBASE + 0x00215000))
#define REGS_UART ((struct UartRegs *)(PBASE + 0x00201000))