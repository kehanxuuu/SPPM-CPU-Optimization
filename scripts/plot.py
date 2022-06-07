import matplotlib.pyplot as plt
import matplotlib
import csv
import os

matplotlib.style.use('ggplot')

file_loc = "../out/results/pre/data"

stages = [0, 1, 2, 3, 4, 5]
param_types = ["iter", "photon", "size"]
param_col_names = ["iterations", "photons", "image_size"]
y_col_name = "cycles"
file_path = os.path.join(file_loc, "stage-{}_{}.csv")
scenes = ["cornell", "large", "mirror", "random", "surgery"]
coords = [[0, 0], [0, 1], [0, 2], [1, 0], [1, 1]]
coords_remove = [[1, 2]]

for i in range(len(param_types)):
    param_type = param_types[i]
    param_col_name = param_col_names[i]
    fig, axs = plt.subplots(2, 3, sharex=True)

    to_get_path = file_path.format(stages[0], param_type)
    base_xy = {}
    for scene in scenes:
        base_xy[scene] = ([], [])
    with open(to_get_path) as to_get_file:
        data = csv.DictReader(to_get_file)
        for line in data:
            base_xy[line["scene"]][0].append(int(line[param_col_name]))
            base_xy[line["scene"]][1].append(int(line[y_col_name]))


    for stage in stages:
        print(stage)
        to_get_path = file_path.format(stage, param_type)
        xy = {}
        for scene in scenes:
            xy[scene] = ([], [])

        with open(to_get_path) as to_get_file:
            data = csv.DictReader(to_get_file)
            for line in data:
                xy[line["scene"]][0].append(int(line[param_col_name]))
                xy[line["scene"]][1].append(int(line[y_col_name]))
            for j in range(len(scenes)):
                scene = scenes[j]
                coord = coords[j]
                axs[coord[0], coord[1]].set_xscale('log')
                # axs[coord[0], coord[1]].set_yscale('log')
                speedup_y = list(base_xy[scene][1][k] / xy[scene][1][k] for k in range(len(base_xy[scene][1])))
                axs[coord[0], coord[1]].plot(xy[scene][0], speedup_y, label=f"Opt {stage}")

    for coord in coords_remove:
        fig.delaxes(axs[coord[0], coord[1]])

    axs.flatten()[-2].legend(loc='upper center', bbox_to_anchor=(1.5, 1), ncol=1)
    fig.set_size_inches(12, 8)

    fig.savefig(f"{param_type}.png", dpi=700)

