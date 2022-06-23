import subprocess
import re
from tqdm import tqdm

exe = "/Users/james/ASL/team32/cmake-build-release/src/main"
repeat = 10

D = []
S = []
L = [2 ** i for i in range(13, 21)]
W = []
H = []
R = []
cam_t = []
bdl_t = []
pho_t = []
con_t = []

for l in tqdm(L):
    ct, bt, pt, ot = 0, 0, 0, 0
    for i in range(repeat):
        program = subprocess.run([exe, '-p', f'{l}', '-i', '1'], capture_output=True)
        stdout, stderr = bytes.decode(program.stdout), bytes.decode(program.stderr)


        def get_int(regex):
            mt = re.search(regex, stdout)
            return int(mt.group(1))


        if i == 0:
            S.append(get_int("Initializing a scene with (\d+) objects"))
            D.append(get_int("ray_max_depth = (\d+)"))
            W.append(get_int("width = (\d+)"))
            H.append(get_int("height = (\d+)"))
            mt = re.search("initial_radius = (\d+.\d+)", stdout)
            R.append(float(mt.group(1)))

        lines = stderr.split('\n')


        def get_clocks(item):
            line = [l for l in lines if item in l][0]
            mt = re.search("(\d+\.\d+) G clocks", line)
            return float(mt.group(1))


        ct += get_clocks("Camera pass")
        bt += get_clocks("Build lookup")
        pt += get_clocks("Photon pass")
        ot += get_clocks("Consolidate")

    cam_t.append(ct / repeat)
    bdl_t.append(bt / repeat)
    pho_t.append(pt / repeat)
    con_t.append(ot / repeat)

for lis in [S, D, W, H, R]:
    for a in lis:
        assert a == lis[0]

with open('photon experiment.csv', 'w') as f:
    f.write('l, camera pass, build lookup, photon pass, consolidate\n')
    for l, ct, bt, pt, ot in zip(L, cam_t, bdl_t, pho_t, con_t):
        f.write(f"{l}, {ct}, {bt}, {pt}, {ot}\n")
    f.write('\n')
    f.write('default params, p, d, |S|, initial radius\n')
    f.write(f"value, {W[0]} x {H[0]}, {D[0]}, {S[0]}, {R[0]}\n")
