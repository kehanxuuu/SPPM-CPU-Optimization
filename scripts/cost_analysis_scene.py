import subprocess
import re
from tqdm import tqdm

exe = "/Users/james/ASL/team32/cmake-build-release/src/main"
repeat = 10

scenes = ['cornell', 'large', 'mirror', 'random', 'surgery']
D = []
S = []
L = []
W = []
H = []
R = []
cam_t = []
bdl_t = []
pho_t = []
con_t = []

for scene in tqdm(scenes):
    ct, bt, pt, ot = 0, 0, 0, 0
    for i in range(repeat):
        program = subprocess.run([exe, '-s', scene, '-i', '1'], capture_output=True)
        stdout, stderr = bytes.decode(program.stdout), bytes.decode(program.stderr)


        def get_int(regex):
            mt = re.search(regex, stdout)
            return int(mt.group(1))


        if i == 0:
            D.append(get_int("ray_max_depth = (\d+)"))
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

for lis in [D, L, W, H, R]:
    for a in lis:
        assert a == lis[0]

with open('scene experiment.csv', 'w') as f:
    f.write('scene, s, camera pass, build lookup, photon pass, consolidate\n')
    for scene, s, ct, bt, pt, ot in zip(scenes, S, cam_t, bdl_t, pho_t, con_t):
        f.write(f"{scene}, {s}, {ct}, {bt}, {pt}, {ot}\n")
    f.write('\n')
    f.write('default params, p, l, d, initial radius\n')
    f.write(f"value, {W[0]} x {H[0]}, {L[0]}, {D[0]}, {R[0]}\n")
