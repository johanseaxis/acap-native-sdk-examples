import argparse
import glob
import numpy as np
import os
import tensorflow as tf

from PIL import Image

parser = argparse.ArgumentParser(description='Converts a SavedModel to \
                                              .tflite with INT8 quantization.')
parser.add_argument('-i', '--input', type=str, required=True,
                    help='path to the SavedModel to convert')
parser.add_argument('-d', '--dataset', type=str, required=True,
                    help='path to the dataset directory')
parser.add_argument('-o', '--output', type=str, default='converted_model.tflite',
                    help='path to output the .tflite model to')

args = parser.parse_args()


# !! IMPORTANT !! You need to define this generator yourself to fit your model!
# A data generator which produces samples from the model's domain
# Calling yield on this generator should output samples of the same type and
# shape as the inputs to the model
def a_representative_datagenerator():
    # List the samples to yield
    samples = glob.glob(os.path.join(args.dataset, '*'))
    for sample_path in samples:
        # Load and preprocess the input data
        sample = Image.open(sample_path).resize((256, 256))
        if sample.mode != 'RGB':
            continue
        preprocessed_sample = np.array(sample, dtype=np.float32) / 255.
        preprocessed_sample = np.expand_dims(preprocessed_sample, axis=0)
        yield [preprocessed_sample]


# Create the converter. As we're converting a model of the
# SavedModel format, we're using the from_saved_model function
converter = tf.lite.TFLiteConverter.from_saved_model(args.input)

# Flags which set what optimizations to perform. The DEFAULT flag
# enables quantization of all fixed parameters, such as weights
converter.optimizations = [tf.lite.Optimize.DEFAULT]

# Set the converter to use the data generator defined above
converter.representative_dataset = a_representative_datagenerator

# Ensure that if any ops can't be quantized, the converter throws an error
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.target_spec.supported_types = [tf.int8]

# Set the input and output tensors to unsigned INT8
converter.inference_input_type = tf.uint8
converter.inference_output_type = tf.uint8

# Perform the conversion
tflite_model = converter.convert()

# Write the converted model to disk
open(args.output, "wb").write(tflite_model)
