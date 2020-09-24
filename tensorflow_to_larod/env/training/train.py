import argparse
import tensorflow as tf
from tensorflow.keras.losses import BinaryCrossentropy
from model import create_model
from utils import DataGenerator


def weighted_bce(weights={0: 0.5, 1: 0.5}):
    def loss(y_true, y_pred):
        bce_obj = BinaryCrossentropy(reduction=tf.keras.losses.Reduction.NONE)
        class_zero_w = tf.ones_like(y_true, dtype=tf.float32) * weights[0]
        class_one_w = tf.ones_like(y_true, dtype=tf.float32) * weights[1]
        weight_mat = tf.keras.backend.switch(y_true, class_one_w, class_zero_w)
        bce_loss = bce_obj(y_true, y_pred)
        weighted_loss = tf.math.multiply(bce_loss, weight_mat)
        return tf.math.reduce_mean(weighted_loss)
    return loss

def train(image_dir, annotation_path):
    person_car_indicator = create_model()
    # Set weights for each output as the data is quite imbalanced
    person_weights = {0: 2, 1: 2}
    car_weights = {0: 1, 1: 10}

    person_car_indicator.compile(optimizer='adam', metrics=['binary_accuracy'],
                                 loss=[weighted_bce(weights=person_weights),
                                       weighted_bce(weights=car_weights)])
    person_car_indicator.summary()
    data_generator = DataGenerator(image_dir, annotation_path, batch_size=64)
    person_car_indicator.fit(data_generator, epochs=5)

    tf.saved_model.save(person_car_indicator, 'saved_model')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Train a basic model.')
    parser.add_argument('-i', '--images', type=str, required=True,
                        help='path to the directory containing training images')
    parser.add_argument('-a', '--annotations', type=str, required=True,
                        help='path to the .json-file containing COCO instance annotations')

    args = parser.parse_args()
    train(args.images, args.annotations)
