from tensorflow.keras.layers import *
from tensorflow.keras.models import Model


def _residual_block(x, n_filters, strides=1):
    shortcut = x

    x = Conv2D(n_filters, 3, strides=strides, padding='same')(x)
    x = BatchNormalization()(x)
    x = Activation('relu')(x)

    x = Conv2D(n_filters, 3,  padding='same')(x)
    x = BatchNormalization()(x)

    shortcut = Conv2D(n_filters, 1, strides=strides, padding='same')(shortcut)
    shortcut = BatchNormalization()(shortcut)

    x = Add()([shortcut, x])
    x = Activation('relu')(x)

    return x


def create_model(blocks=5, kernel_size=3):
    img_in = Input(shape=(256, 256, 3))

    x = img_in
    n_filters = 8
    for _ in range(blocks):
        x = _residual_block(x, n_filters)
        n_filters *= 2
        x = _residual_block(x, n_filters, strides=2)
    x = Conv2D(256, 3, strides=2)(x)
    x = Activation('relu')(x)
    x = Flatten()(x)

    x = Dense(64)(x)
    x = Activation('relu')(x)
    person_pred = Dense(1, activation='sigmoid', name='A_person_pred')(x)
    car_pred = Dense(1, activation='sigmoid', name='B_car_pred')(x)

    return Model(img_in, [person_pred, car_pred], name='person_car_indicator')
