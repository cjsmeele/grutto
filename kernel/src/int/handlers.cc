/* Copyright (c) 2018, Chris Smeele
 *
 * This file is part of Grutto.
 *
 * Grutto is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Grutto is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Grutto.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "handlers.hh"
#include "pic.hh"

struct interrupt_frame;

namespace Int {

    void (*ticker)(interrupt_frame*) = nullptr;
}

static void dump_frame(OStream &o, Int::interrupt_frame &frame) {
    // This would need to be changed for amd64, of course.
    static_assert(sizeof(void*) == 4);

    auto fmtone = [&] (const char *name, u32 v) {
        o.fmt("{-3} {08x}  ", name, v);
    };

    fmtone("eax", frame.eax); fmtone("cs", frame.cs); fmtone("eip", frame.eip); o.put_char('\n');
    fmtone("ebx", frame.ebx); fmtone("ss", frame.ss); fmtone("esp", frame.esp); o.put_char('\n');
    fmtone("ecx", frame.ecx); fmtone("ds", frame.ds); fmtone("ebp", frame.ebp); o.put_char('\n');
    fmtone("edx", frame.edx); fmtone("es", frame.es); fmtone("cr0", asm_cr0()); o.put_char('\n');
    fmtone("esi", frame.esi); fmtone("fs", frame.fs); fmtone("cr2", asm_cr2()); o.put_char('\n');
    fmtone("edi", frame.edi); fmtone("gs", frame.gs); fmtone("cr3", asm_cr3()); o.put_char('\n');
}

extern "C" {

    struct Exception {
        const char *name;
        const char *desc;
        bool has_error;
    };
    const Exception exceptions[32] = {
        /* 0x00 */ { "#DE"    , "Divide Error Exception"         , false } ,
        /* 0x01 */ { "#DB"    , "Debug Exception"                , false } ,
        /* 0x02 */ {  nullptr , "Non-Maskable Interrupt"         , false } ,
        /* 0x03 */ { "#BP"    , "Breakpoint Exception"           , false } ,
        /* 0x04 */ { "#OF"    , "Overflow Exception"             , false } ,
        /* 0x05 */ { "#BR"    , "BOUND Range Exceeded Exception" , false } ,
        /* 0x06 */ { "#UD"    , "Invalid Opcode Exception"       , false } ,
        /* 0x07 */ { "#NM"    , "Device Not Available Exception" , false } ,
        /* 0x08 */ { "#DF"    , "Double Fault Exception"         , true  } ,
        /* 0x09 */ {  nullptr , "Coprocessor Segment Overrun"    , false } ,
        /* 0x0a */ { "#TS"    , "Invalid TSS Exception"          , true  } ,
        /* 0x0b */ { "#NP"    , "Segment Not Present"            , true  } ,
        /* 0x0c */ { "#SS"    , "Stack Fault Exception"          , true  } ,
        /* 0x0d */ { "#GP"    , "General Protection Exception"   , true  } ,
        /* 0x0e */ { "#PF"    , "Page-Fault Exception"           , true  } ,
        /* 0x0f */ {  nullptr , "Unknown Exception"              , false } ,
        /* 0x10 */ { "#MF"    , "FPU Floating-Point Error"       , false } ,
        /* 0x11 */ { "#AC"    , "Alignment Check Exception"      , true  } ,
        /* 0x12 */ { "#MC"    , "Machine-Check Exception"        , false } ,
        /* 0x13 */ { "#XM"    , "SIMD Floating-Point Exception"  , false } ,
        /* 0x14 */ {  nullptr , "Unknown Exception"              , false } ,
        /* 0x15 */ {  nullptr , "Unknown Exception"              , false } ,
        /* 0x16 */ {  nullptr , "Unknown Exception"              , false } ,
        /* 0x17 */ {  nullptr , "Unknown Exception"              , false } ,
        /* 0x18 */ {  nullptr , "Unknown Exception"              , false } ,
        /* 0x19 */ {  nullptr , "Unknown Exception"              , false } ,
        /* 0x1a */ {  nullptr , "Unknown Exception"              , false } ,
        /* 0x1b */ {  nullptr , "Unknown Exception"              , false } ,
        /* 0x1c */ {  nullptr , "Unknown Exception"              , false } ,
        /* 0x1d */ {  nullptr , "Unknown Exception"              , false } ,
        /* 0x1e */ {  nullptr , "Unknown Exception"              , false } ,
        /* 0x1f */ {  nullptr , "Unknown Exception"              , false }
    };


    //void common_exception_handler(Int::interrupt_frame *frame) __attribute__((noinline));
    void common_exception_handler(Int::interrupt_frame *frame) {

        koi(LL::critical).fmt
           ("\n>>> exception {2} ({02x}:{02x}: {}) at {08x}\n\n"
           ,exceptions[frame->int_no].name
           ,frame->int_no
           ,frame->err_code
           ,exceptions[frame->int_no].desc
           ,frame->eip);

        if (frame->int_no == 0x0e) {
            auto err = frame->err_code;
            koi(LL::critical).fmt
               ("A page fault was triggered while {} vma {} from {}.\n"
                "{}\n\n"
               ,err & 2 ? "writing" : "reading"
               ,vaddr_t{asm_cr2()}
               ,err & 4 ? "user-mode code" : "kernel-mode code"
               ,err & 1 ? "Page-level protection bits prevented the access."
                        : "The page was NOT present at the time.");
        }

        dump_frame(koi(LL::critical), *frame);

        panic();
    }

    //void common_interrupt_handler(Int::interrupt_frame *frame) __attribute__((noinline));
    void common_interrupt_handler(Int::interrupt_frame *frame) {
        //dink('z');
        auto int_no = frame->int_no;
        if (frame->int_no == 0x20) {
            ++Time::systick_counter;
            if (Int::ticker && Time::systick_counter % 10 == 0)
                Int::ticker(frame);
        } else {
            koi(LL::warning).fmt("\n>>> unhandled irq {#04x}\n", frame->int_no);
        }

        //Int::Pic::ack(frame->int_no >= 0x28 && frame->int_no < 0x30);
        Int::Pic::ack(int_no >= 0x28 && int_no < 0x30);
        //Int::sti();
    }
}
