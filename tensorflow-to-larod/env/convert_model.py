""" convert_model.py
    Converts a saved Tensorflow SavedModel model to the Tensorflow Lite
    format. The conversion is also set up to quantize the model to the INT8
    precision, which allows usage on INT8 compatible devices.

    Usage: python convert_model.py -i <SavedModel path> \
        -d <dataset directory for data generation> [-o <.tflite output path>]
"""
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
parser.add_argument('-o', '--output', type=str,
                    default='converted_model.tflite',
                    help='path to output the .tflite model to')

args = parser.parse_args()


# !! IMPORTANT !! You need to define this generator yourself to fit your model!
def a_representative_datagenerator():
    """ A data generator which produces samples from the model's domain.
        Calling yield on this generator should output samples of the same type
        and shape as the inputs to the model, similar to those it has been
        trained on.

    Yields:
        np.float32 array: An RGB image from the dataset directory, which has
            been processed like the images the model has been
            trained on. In this case, this includes
            normalization and resizing. The output array has the
            shape (1, 256, 256, 3).
    """
    samples = glob.glob(os.path.join(args.dataset, '*'))
    for sample_path in samples:
        sample = Image.open(sample_path).resize((256, 256))
        if sample.mode != 'RGB':
            continue
        preprocessed_sample = np.array(sample, dtype=np.float32) / 255.
        preprocessed_sample = np.expand_dims(preprocessed_sample, axis=0)
        yield [preprocessed_sample]


# Create the converter. As the model to convert is of the
# SavedModel format, the from_saved_model function is used
converter = tf.lite.TFLiteConverter.from_saved_model(args.input)

# Flags which set what optimizations to perform. The DEFAULT flag
# enables quantization of all fixed parameters, such as weights
converter.optimizations = [tf.lite.Optimize.DEFAULT]

# Set the converter to use the data generator defined above
converter.representative_dataset = a_representative_datagenerator

# Select the set of operators to use
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]

# Set the desired datatype for inputs and outputs
converter.inference_input_type = tf.uint8
converter.inference_output_type = tf.uint8

# Perform the conversion
tflite_model = converter.convert()

# Write the converted model to disk
open(args.output, "wb").write(tflite_model)
