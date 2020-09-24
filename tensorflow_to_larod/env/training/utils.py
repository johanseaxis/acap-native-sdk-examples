import tensorflow as tf
import numpy as np
import glob
import json
import os

from PIL import Image

class DataGenerator(tf.keras.utils.Sequence):
    def __init__(self, samples_dir, annotation_path, data_shape=(256, 256), batch_size=16, shuffle=True, float_input=True):
        self.samples_dir = samples_dir
        annotations = json.load(open(annotation_path, 'r'))
        self.annotations = self._reprocess_annotations(annotations)
        self.data_shape = data_shape
        self.batch_size = batch_size
        self.shuffle = shuffle
        self.float_input = float_input
        self.on_epoch_end()

    def __len__(self):
        'Denotes the number of batches per epoch'
        return int(np.floor((len(self.annotations) / self.batch_size)))

    def __getitem__(self, index):
        'Generate one batch of data'
        temp_indices = self.indices[index*self.batch_size:(index+1)*self.batch_size]
        temp_annotations = [self.annotations[k] for k in temp_indices]
        X, y  = self.__data_generation(temp_annotations)
        return X, y

    def on_epoch_end(self):
        'Updates indexes after each epoch'
        self.indices = np.arange(len(self.annotations))
        if self.shuffle == True:
            np.random.shuffle(self.indices)

    def _reprocess_annotations(self, annotations):
        has_person = set()
        has_car = set()
        for annotation in annotations['annotations']:
            if annotation['category_id'] == 1:
                has_person.add(annotation['image_id'])
            elif annotation['category_id'] == 3:
                has_car.add(annotation['image_id'])

        processed_annotations = []
        for image in annotations['images']:
            sample = {'file_name': image['file_name'],
                      'id': image['id'],
                      'has_car': image['id'] in has_car,
                      'has_person': image['id'] in has_person}
            img_path = os.path.join(self.samples_dir, image['file_name'])
            file_exists = os.path.exists(img_path)

            if file_exists and Image.open(img_path).mode == 'RGB':
                processed_annotations.append(sample)

        return processed_annotations

    def __data_generation(self, temp_annotations):
        'Generates data containing batch_size samples'
        # Define our input and outputs
        X = np.zeros((self.batch_size, *self.data_shape + (3,)), dtype=np.uint8)
        y_person = np.zeros((self.batch_size, 1), dtype=np.uint8)
        y_car = np.zeros((self.batch_size, 1), dtype=np.uint8)

        for i, annotation in enumerate(temp_annotations):
            img_path = os.path.join(self.samples_dir, annotation['file_name'])
            img = Image.open(img_path).resize(self.data_shape)
            # Horizontal flipping with p=0.5
            if np.random.random() >= 0.5:
                img = img.transpose(Image.FLIP_LEFT_RIGHT)
            X[i,] = np.array(img)
            y_person[i,] = annotation['has_person']
            y_car[i,] = annotation['has_car']

        if self.float_input:
            X = X.astype(np.float32) / 255.
        return X, (y_person, y_car)
