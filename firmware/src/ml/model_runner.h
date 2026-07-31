/**
 * @file    model_runner.h
 * @brief   SchedTiny TinyML model runner abstraction layer.
 *
 * Platform-independent interface between the scheduling/task layer and
 * TensorFlow Lite Micro (TFLM) / CMSIS-NN. All model-specific details
 * are hidden in model_kws.c and model_anomaly.c.
 *
 * The scheduler calls model_runner_invoke() without knowing which model
 * is loaded. The model is selected at compile time via schedtiny_config.h.
 *
 * @see     docs/SPEC.md REQ-BENCH-003
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_MODEL_RUNNER_H
#define SCHEDTINY_MODEL_RUNNER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum number of output classes. */
#define MODEL_MAX_CLASSES   (35U)

/**
 * @brief   Model runner status codes.
 */
typedef enum
{
    MODEL_OK              = 0,
    MODEL_ERR_NOT_INIT    = 1,
    MODEL_ERR_INVOKE      = 2,
    MODEL_ERR_INPUT_SIZE  = 3,
} ModelStatus_t;

/**
 * @brief   Inference result structure.
 */
typedef struct
{
    uint8_t  predicted_class;              /**< Index of the highest-scoring class */
    uint8_t  scores[MODEL_MAX_CLASSES];    /**< INT8 output scores (all classes) */
    uint32_t latency_us;                   /**< Inference latency measured by this layer */
} ModelResult_t;

/**
 * @brief   Initialize the TFLM interpreter and load the model.
 *
 * Allocates tensor arena from the static buffer defined in schedtiny_config.h.
 * Must be called once before any inference.
 *
 * @return  MODEL_OK on success.
 */
ModelStatus_t model_runner_init(void);

/**
 * @brief   Run one forward pass of inference on pre-loaded input data.
 *
 * Input data must be written to the input tensor BEFORE calling this function.
 * Use model_runner_get_input_buffer() to get the pointer.
 *
 * @param[out] result  Pointer to ModelResult_t to populate.
 * @return             MODEL_OK on success.
 */
ModelStatus_t model_runner_invoke(ModelResult_t *result);

/**
 * @brief   Return a pointer to the model's input tensor buffer.
 *
 * @param[out] size  Set to the input tensor size in bytes.
 * @return           Pointer to the input tensor data buffer, or NULL on error.
 */
int8_t *model_runner_get_input_buffer(uint32_t *size);

/**
 * @brief   Return the model's input tensor size in bytes.
 * @return  Input tensor size in bytes.
 */
uint32_t model_runner_get_input_size(void);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_MODEL_RUNNER_H */
