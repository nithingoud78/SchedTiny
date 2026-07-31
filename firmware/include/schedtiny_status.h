/**
 * @file    schedtiny_status.h
 * @brief   Global status and error codes for SchedTiny.
 *
 * Provides a standardized set of return values across the framework.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_STATUS_H
#define SCHEDTINY_STATUS_H

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief   Standard status codes.
     */
    typedef enum
    {
        SCHED_OK              = 0, /**< Operation successful */
        SCHED_ERR_PARAM       = 1, /**< Invalid parameter provided */
        SCHED_ERR_STATE       = 2, /**< Invalid state for operation */
        SCHED_ERR_OVERFLOW    = 3, /**< Buffer or counter overflow */
        SCHED_ERR_TIMEOUT     = 4, /**< Operation timed out */
        SCHED_ERR_NOT_FOUND   = 5, /**< Resource not found */
        SCHED_ERR_UNSUPPORTED = 6, /**< Feature not supported or implemented */
        SCHED_ERR_NO_MEM      = 7  /**< Insufficient memory or registry full */
    } SchedStatus_t;

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_STATUS_H */
