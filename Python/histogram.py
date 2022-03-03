import sys
import matplotlib.pyplot as plt
import skimage.io
import numpy as np


def main():
    if len(sys.argv) < 3:
        print("Uso: " + sys.argv[0] + " ruta_imagen" + "pos_vertical_line")
        return

    line = sys.argv[2]
    image = skimage.io.imread(fname=sys.argv[1]) #, as_gray=True)
    plt.hist(image.flatten(), bins=128, range=[0,255], color='black')

    # plt.axvline(x=line)
    # plt.gca().set_ylim(0, 250000)# 350000)
    plt.gca().get_yaxis().set_visible(False)
    plt.show()


if __name__ == "__main__":
    main()