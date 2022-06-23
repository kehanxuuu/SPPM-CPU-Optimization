import subprocess
import re
from tqdm import tqdm

exe = "/Users/james/ASL/team32/cmake-build-release/src/main"
repeat = 10

D = list(range(1, 9))
S = []
L = []
W = []
H = []
R = []
cam_t = []
bdl_t = []
pho_t = []
con_t = []

for d in tqdm(D):
    ct, bt, pt, ot = 0, 0, 0, 0
    for i in range(repeat):
        program = subprocess.run([exe, '-d', f'{d}', '-i', '1'], capture_output=True)
        stdout, stderr = bytes.decode(program.stdout), bytes.decode(program.stderr)


        def get_int(regex):
            mt = re.search(regex, stdout)
            return int(mt.group(1))


        if i == 0:
            S.append(get_int("Initializing a scene with (\d+) objects"))
            L.append(get_int("photon_num_per_iter = (\d+)"))
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

for lis in [S, L, W, H, R]:
    for a in lis:
        assert a == lis[0]

with open('depth experiment.csv', 'w') as f:
    f.write('d, camera pass, build lookup, photon pass, consolidate\n')
    for d, ct, bt, pt, ot in zip(D, cam_t, bdl_t, pho_t, con_t):
        f.write(f"{d}, {ct}, {bt}, {pt}, {ot}\n")
    f.write('\n')
    f.write('default params, p, l, |S|, initial radius\n')
    f.write(f"value, {W[0]} x {H[0]}, {L[0]}, {S[0]}, {R[0]}\n")
