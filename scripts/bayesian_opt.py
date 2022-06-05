from skopt import gp_minimize, dump
from skopt.space.space import Real, Categorical
import numpy as np
from subprocess import DEVNULL, STDOUT, check_call
import os
from time import time

i = 0


def res_callback(res):
    global i
    dump(res, f'temp_res.pkl')
    print(i)
    print(res)
    i += 1


def obj_fun(inps):
    radius_mult = inps[0]
    radius_type = inps[1]
    with open(os.devnull, 'wb') as devnull:
        check_call(["cmake", ".", "-DCMAKE_C_COMPILER=/usr/bin/gcc",
                   f"-DCMAKE_BUILD_TYPE=Profile", f"-D_SPPM_RADIUS_MULT={float(radius_mult)}"], stdout=devnull, stderr=devnull)
        check_call(["make", "main", f"-j10", "VERBOSE=1"], stdout=devnull, stderr=devnull)
        start = time() 
        try:
            check_call(["./src/main"], stdout=devnull, stderr=devnull, timeout=100)
        except:
            pass
        end = time()
        return end - start


build_directory_name = "bayesian_opt_build"

if(not os.path.exists(build_directory_name)):
    os.mkdir(build_directory_name)
os.chdir(build_directory_name)

res = gp_minimize(obj_fun,
                  [Real(0.01, 20.0), Categorical([0, 1])],
                  x0=[2, 0],
                  n_calls=200,
                  callback=res_callback,
                  random_state=42)

print(res)
dump(res, 'result.pkl')