#include "task.h"

#include "FreeRTOS.h"

#include <cmocka.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

void vTaskStartScheduler(void)
{
    function_called();
}
