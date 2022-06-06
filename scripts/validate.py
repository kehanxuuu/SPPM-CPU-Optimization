import subprocess
import os
import sys
import csv
import argparse

cmd1 = ['sudo', 'perf', 'stat', './src/main']
cmd2 = ['OPENCV_IO_ENABLE_OPENEXR=1', 'python3', 'scripts/compute_dssim.py']

def validate():
    results = []
    for scene in ['cornell', 'large', 'mirror', 'random', 'surgery']:
        num_iter = 1
        for _ in range(11):
            num_iter *= 2
            images = []
            for algo in ['sppm', 'sppm-simd']:
                print(f"{scene=}, {algo=}")
        
                img_out = f"{scene}_{algo}.exr"
                os.system('echo 3 | sudo tee /proc/sys/vm/drop_caches >> /dev/null')
                subprocess.run(cmd1 + 
                    ['--iterations', str(num_iter)] + 
                    ['--algorithm', algo] + 
                    ['--scene', scene] + 
                    [img_out], 
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.STDOUT)
                images.append(img_out)
            
            out = subprocess.getoutput(' '.join(cmd2 + images))

            dssim = float(out)

            print('iterations', num_iter)
            print(f'{dssim=}\n')

            results.append({
                'scene': scene,
                'iterations': num_iter,
                'dssim': dssim,
            })
    return dump_results(results)

def dump_results(results):
    with open(f'out/validation.csv', "w", newline="") as f:
        headers = results[0].keys()
        cw = csv.DictWriter(f, headers, delimiter=',', quotechar='|', quoting=csv.QUOTE_MINIMAL)
        cw.writeheader()
        cw.writerows(results)
    return

if __name__ == '__main__':
    sys.exit(validate())