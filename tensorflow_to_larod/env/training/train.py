import argparse
import tensorflow as tf
from model import create_model
from utils import DataGenerator


def train(image_dir, annotation_path):
    person_car_indicator = create_model()
    person_car_indicator.compile(optimizer='adam', metrics=['binary_accuracy'],
                                 loss=['bce', 'bce'])
    person_car_indicator.summary()
    data_generator = DataGenerator(image_dir, annotation_path, batch_size=32)
    person_car_indicator.fit(data_generator, epochs=5)

    tf.saved_model.save(person_car_indicator, 'models/saved_model')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Train a basic model.')
    parser.add_argument('-i', '--images', type=str, required=True,
                        help='path to the directory containing training \
                        images')
    parser.add_argument('-a', '--annotations', type=str, required=True,
                        help='path to the .json-file containing COCO instance \
                        annotations')

    args = parser.parse_args()
    train(args.images, args.annotations)
