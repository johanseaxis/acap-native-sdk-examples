from tensorflow.keras.layers import *
from tensorflow.keras.models import Model


def _residual_block(x, n_filters):
    shortcut = x

    x = Conv2D(n_filters, 3, strides=2, padding='same')(x)
    x = BatchNormalization()(x)
    x = Activation('relu')(x)

    x = Conv2D(n_filters, 3,  padding='same')(x)
    x = BatchNormalization()(x)

    shortcut = Conv2D(n_filters, 1, strides=2, padding='same')(shortcut)
    shortcut = BatchNormalization()(shortcut)

    x = Add()([shortcut, x])
    x = Activation('relu')(x)

    return x

def create_model(blocks=5, kernel_size=3):
    img_in = Input(shape=(256, 256, 3))

    x = img_in
    n_filters = 16
    for _ in range(blocks):
        x = _residual_block(x, n_filters)
        n_filters *= 2

    x = GlobalAveragePooling2D()(x)
    x = Dense(64, activation='relu')(x)
    person_pred = Dense(1, activation='sigmoid', name='person_pred')(x)
    car_pred = Dense(1, activation='sigmoid', name='car_pred')(x)

    return Model(img_in, [person_pred, car_pred], name='person_car_indicator')
