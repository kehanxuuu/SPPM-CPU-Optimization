import sys
import numpy as np
from numbers import Integral
from scipy import ndimage as ndi
import cv2

"""
Run the following:

OPENCV_IO_ENABLE_OPENEXR=1 python3 scripts/compute_dssim.py out1.exr out2.exr

"""

def apply_gaussian_kernel(image, sigma, mode, truncate):

    sigma = [sigma] * (image.ndim - 1)
    sigma = list(sigma)

    channel_axis = -1
    sigma.insert(channel_axis % image.ndim, 0)

    image = image.astype(np.float64, copy=False)

    return ndi.gaussian_filter(image, sigma, 
        mode=mode, cval=0, truncate=truncate)

def get_dssim(img1, img2):
    """
    Ref:
    * https://ece.uwaterloo.ca/~z70wang/publications/ssim.pdf
    * https://arxiv.org/abs/0901.0065
    * https://arxiv.org/pdf/2202.02616.pdf
    """

    if not img1.shape == img2.shape:
        raise ValueError("Images don't have the same dimensions.")
        
    truncate = 3.5 # Wang et. al. 2004.
    sigma = 1.5
    window = 11 # Baker et. al. 2022.

    filter_args = {
        'sigma': sigma, 
        'truncate': truncate, 
        'mode': 'reflect' # 'nearest'
    }


    # Calculate weighted means.
    ux = apply_gaussian_kernel(img1, **filter_args)
    uy = apply_gaussian_kernel(img2, **filter_args)

    # Calculate weighted variances and covarances.
    cov = 1.0  # From Wang et. al. 2004.
    ux_ = apply_gaussian_kernel(img1 * img1, **filter_args)
    uy_ = apply_gaussian_kernel(img2 * img2, **filter_args)
    uxy = apply_gaussian_kernel(img1 * img2, **filter_args)
    xv = cov * (ux_ - ux * ux)
    yv = cov * (uy_ - uy * uy)
    xyv = cov * (uxy - ux * uy)

    '''SSIM params.'''
    # R = 4095
    # K1 = 0.01
    # K2 = 0.03
    # C1 = (K1 * R) ** 2
    # C2 = (K2 * R) ** 2

    C1 = 1e-8 # Baker et. al. 2022.
    C2 = 1e-8

    A, A_, B, B_ = (
        2 * ux * uy + C1, 
        2 * xyv + C2, 
        ux ** 2 + uy ** 2 + C1, 
        xv + yv + C2
    )
    D = B * B_
    S = (A * A_) / D

    dssim_avg = S.mean(dtype=np.float64)

    # print(f"{S.shape=}, {window=}")
    return dssim_avg


if __name__ == "__main__":
    path1, path2 = sys.argv[1:]
    img1 = cv2.imread(path1)
    img2 = cv2.imread(path2)

    # print(f"{img1.shape=}")
    # print(f"{img2.shape=}")

    print(f"{get_dssim(img1, img2): 0.5f}")
    # print(f"\nDSSIM{path1, path2}={get_dssim(img1, img2)}")
