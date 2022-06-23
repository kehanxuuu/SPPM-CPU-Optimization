import subprocess
import re
from tqdm import tqdm

exe = "/Users/james/ASL/team32/cmake-build-release/src/main"
repeat = 10

D = []
S = []
L = []
W = [2 ** i for i in range(5, 12)]
H = [w * 3 // 4 for w in W]
P = []
R = []
cam_t = []
bdl_t = []
pho_t = []
con_t = []

for (w, h) in tqdm(zip(W, H)):
    ct, bt, pt, ot = 0, 0, 0, 0
    for i in range(repeat):
        program = subprocess.run([exe, '-w', f'{w}', '-h', f'{h}', '-i', '1'], capture_output=True)
        stdout, stderr = bytes.decode(program.stdout), bytes.decode(program.stderr)


        def get_int(regex):
            mt = re.search(regex, stdout)
            return int(mt.group(1))


        if i == 0:
            D.append(get_int("ray_max_depth = (\d+)"))
            S.append(get_int("Initializing a scene with (\d+) objects"))
            L.append(get_int("photon_num_per_iter = (\d+)"))
            P.append(w * h)
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

for lis in [D, S, L, R]:
    for a in lis:
        assert a == lis[0]

with open('pixel experiment.csv', 'w') as f:
    f.write('p, w, h, camera pass, build lookup, photon pass, consolidate\n')
    for p, w, h, ct, bt, pt, ot in zip(P, W, H, cam_t, bdl_t, pho_t, con_t):
        f.write(f"{p}, {w}, {h}, {ct}, {bt}, {pt}, {ot}\n")
    f.write('\n')
    f.write('default params, d, l, |S|, initial radius\n')
    f.write(f"value, {D[0]}, {L[0]}, {S[0]}, {R[0]}\n")
