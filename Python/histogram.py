import numpy as np
import skimage.color
import skimage.io
import matplotlib.pyplot as plt
import sys

def main():
    if len(sys.argv) < 2:
        print("Uso: " + sys.argv[0] + " ruta_imagen")
        return

    image = skimage.io.imread(fname=sys.argv[1], as_gray=True)
    plt.imshow(image, cmap = 'gray')
    plt.show()

    # create the histogram
    histogram, bin_edges = np.histogram(image, bins=256, range=(0, 1))
    plt.figure()
    plt.title("Grayscale Histogram")
    plt.xlabel("grayscale value")
    plt.ylabel("pixel count")
    plt.xlim([0.0, 1.0])  # <- named arguments do not work here

    plt.plot(bin_edges[0:-1], histogram)  # <- or here
    plt.show()


if __name__ == "__main__":
    main()