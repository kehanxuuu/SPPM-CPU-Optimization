import subprocess
import os
import sys
import csv
import argparse

cmd = ['sudo', 'perf', 'stat', './src/main', 'out.exr']

def get_cycles(out):
    def find_nth(string, substr, n):
        start = string.find(substr)
        while start >= 0 and n > 1:
            start = string.find(substr, start+len(substr))
            n -= 1
        return start
    return int(''.join(out[find_nth(out, '/sec', 3)+len('/sec'):out.find('cycles')].strip().split(',')))

def run_iter():
    results = []
    num_iter = 1
    for algo in ['pt', 'sppm', 'sppm-simd']:
        print(f"\n{algo=}")
        for _ in range(6):
            num_iter *= 2
            os.system('echo 3 | sudo tee /proc/sys/vm/drop_caches >> /dev/null')
            out = subprocess.getoutput(' '.join(cmd + ['--iterations', str(num_iter)] + ['--algorithm', algo]))
            num_cycles = get_cycles(out)

            print('iterations', num_iter)
            print(f'{num_cycles=}')

            results.append({
                'algorithm': algo,
                'iterations': num_iter,
                'cycles': num_cycles,
            })
    return results

def run_photon():
    results = []
    num_potons = int(12.5e3/2)
    for algo in ['pt', 'sppm', 'sppm-simd']:
        print(f"\n{algo=}")
        for _ in range(7):
            num_potons *= 2
            os.system('echo 3 | sudo tee /proc/sys/vm/drop_caches >> /dev/null')
            out = subprocess.getoutput(' '.join(cmd + ['--photons_per_iter', str(num_potons)] + ['--algorithm', algo]))
            num_cycles = get_cycles(out)

            print('photons_per_iter', num_potons)
            print(f'{num_cycles=}')

            results.append({
                'algorithm': algo,
                'photons': num_potons,
                'cycles': num_cycles,
            })
    return results

def run_size():
    results = []
    h = 24
    w = 32
    for algo in ['pt', 'sppm', 'sppm-simd']:
        print(f"\n{algo=}")
        for _ in range(6):
            h *= 2
            w *= 2
            os.system('echo 3 | sudo tee /proc/sys/vm/drop_caches >> /dev/null')
            out = subprocess.getoutput(' '.join(cmd + ['--height', str(h), '--width', str(w)] + ['--algorithm', algo]))
            num_cycles = get_cycles(out)

            print('height', h, 'width', w)
            print(f'{num_cycles=}')

            results.append({
                'algorithm': algo,
                'image_size': h*w,
                'cycles': num_cycles,
            })
    return results

def main(args):
    if args.xvar == 'size':
        results = run_size()
    elif args.xvar == 'photon':
        results = run_photon()
    else:
        results = run_iter()
    
    print(results)

    with open(f'{args.output}/{args.xvar}.csv', "w", newline="") as f:
        headers = results[0].keys()
        cw = csv.DictWriter(f, headers, delimiter=',', quotechar='|', quoting=csv.QUOTE_MINIMAL)
        cw.writeheader()
        cw.writerows(results)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-x',
        '--xvar',
        required=True,
        metavar='[size, photon, iter]',
        help='Horizontal axis'
    )
    parser.add_argument(
        '-o',
        '--output',
        required=True,
        metavar='path',
        help='Output path for the results'
    )
    args = parser.parse_args()

    sys.exit(main(args))