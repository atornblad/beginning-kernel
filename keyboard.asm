bits 32
section .text

global _keyboard_irq_handler_raw
extern _keyboard_irq_handler

_keyboard_irq_handler_raw:
    call _keyboard_irq_handler
    iretd