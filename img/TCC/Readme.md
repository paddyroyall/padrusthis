#TOPOLOGICAL CLUSTER CLASSIFICATION

This package contains the source code of the Topological Cluster Classification method described in [A. Malins, S. R. Williams, J. Eggers, and C. P. Royall, J. Chem. Phys. 139, 234506 (2013)](http://scitation.aip.org/content/aip/journal/jcp/139/23/10.1063/1.4832897).

##Compilation

From a bash shell, move to the `TCCpackage` folder and type

```
make -C src
```

This will produce the executable `./TCC`.

##Usage

The code performs the analysis of a trajectory file composed by a series of frames in `xyz` format. 

To do so, two main parameter files `inputparameters.in` and `d0_init.params` need to be provided. For non cubic boxes, a geometry input file `d0_box.txt` is also needed in input.

All these scripts can be conveniently generated with the use of the graphical interface `Prepare_Input.py`, which also illustates the possible additional options. A standard `python` installation is needed (including `Tkinter`). To execute the script, just type 


```
python Prepare_Input.py
```
inside the  `TCCpackage` folder.

Once the scripts are generated and bounded to a suitable configuration or trajectory file (see  `ExampleTrajectory.xyz`), just execute

```
./TCC
```

This will provide a series of output files, corresponding to the selections made in the `inputparameters.in` file.

##Input script parameters
This list provides a description of the relevant input parameters in the graphical interface:

#### Trajectory

- **File name**: The trajectory file name.
- **Box type**: The shape of the box: it can be a regulare cube (Cubic), a rectangular parallelepiped (Non-cubic), and a generalised parallelepiped (Triclinic). 
- **Number of particles**: The total number of particles per frame.
- **Number of A particles**: Considering a binary mixture of A and B particles, this enty corresponds to the number of particles of species A. If only one species is present in the model, this number is equal to the total number of particles.
- **Lx, Ly, Lz**: Box lateral sizes.
- **Tilt**: For Triclinic boxes, the geometric tilt of the box (compare with LAMMPS's definition).
- **Number of frames**: The number of frames to be analysed in the trajectory.
- **Start frame**: The initial frame in the sequence of analysed frames.
- **Frame sampling frequency**: The steps between consecutively analysed frames.
- **Alpha relaxation time**: The duration (in simulation time units) of the alpha relaxation time.

#### Bonds

- **Voronoi**: Activate (deactivate) the Voronoi bond detection algorithm. If deactivated, simple (cut-off-based) bond detection is used.
- **PBC**: Activate (deactivate) the usage of Periodic Boundary Conditions. 
- **rcutAA, rcutAB, rcutBB**: Cut-off distances for the AA, AB and BB bonds.
- **Maximum number of bonds**: For memory reasons, bound the maximum number of detected neighbours.
- **Fc parameter**: Modified Voronoi Method (see J. Chem. Phys. 139, 234506 (2013)). For the Wahnstrom binary mixture 0.82 is indicated while for the Kob-Andersen mixture 1 (standard Voronoi) is the suitable value.
- **Binning**: When (and if) computing the bond lengths histograms, this value is used for the binning.

#### Optional output


The default output provides a `*.static_clust` file containing the statistics for the several clusters. Additional output can be set in this section:

- **Bonds file**: A `*.bonds` file containing the list of bonds IDs and lengths for every single particle.
- **All clusters**: For every cluster type, provide a table containing all the clusters, one per row. This takes large HD space.
- **xmol cluster files**: For every cluster, provide an list of N entries distinguishing cluster particles (C-D) from non-cluster ones (A-B).
- **Population per frame**: Provide the statistics of th cluster population for every frame.
- **13A centres**: Centres of the icosahedral structures.
- **Bond length distributions**: Histograms for the several bond longths.
- **Coordination distribution**: Histograms for the number of neighbours.
- **Cluster compositions**: Repartition of A and B particles in the clusters.
- **Coslovich Voronoi-face analysis**: Identify structures on the basis of the number of edges on the Voronoi faces.
 
 
