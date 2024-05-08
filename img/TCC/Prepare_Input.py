import Tkinter as TK, Tkconstants, tkFileDialog
import ttk

from Tkinter import W,E , S,N, LEFT, RIGHT, BOTTOM, TOP, SW,SE, NW,NE, END,NSEW
class TkDialog(TK.Frame):

    def __init__(self, root):

        TK.Frame.__init__(self, root)
        
        self.filevar=TK.StringVar()
        # options for buttons
        button_opt = {'fill': Tkconstants.BOTH, 'padx': 1, 'pady': 1}
        # define buttons
        TK.Button(self, text='open file', command=self.askopenfilename, width=18).pack(**button_opt)
        # define options for opening or saving a file
        self.file_opt = options = {}
        options['defaultextension'] = '.hex'
        options['filetypes'] = [('all files', '.*'), ('text files', '.hex')]
        options['initialfile'] = 'myfile.hex'
        options['parent'] = root
        options['title'] = 'This is a title'

        text_next_to_open_button=TK.Label(mainframe, textvariable=self.filevar)
        self.filevar.set("select a file...")
        text_next_to_open_button.grid(row=0, column=1, sticky=W)

    def askopenfilename(self):

        """Returns an opened file in read mode.
        This time the dialog just returns a filename and the file is opened by your own code.
        """
        # get filename
        filename = tkFileDialog.askopenfilename(**self.file_opt)
        print filename
        print self.filevar.get()
        self.filevar.set(filename)

def printvalue(v):
    print v.get()

import math
if __name__=='__main__':
    root = TK.Tk()
    root.wm_title("PREPARE TCC INPUT SCRIPT")
    mainframe = TK.Frame(root, width = 700, height = 500, relief='groove')
    mainframe.grid(column=3, row=2)
    mainframe.columnconfigure(0, weight=1)
    mainframe.rowconfigure(0, weight=1)
    # Dialog=TkDialog(mainframe)
    # Dialog.grid(row=0,column=0)

    # VARIABLES
    name=TK.StringVar()
    N=TK.IntVar()
    NA=TK.IntVar()
    Lx=TK.DoubleVar()
    Ly=TK.DoubleVar()
    Lz=TK.DoubleVar()
    tilt=TK.DoubleVar()
    TrajectoryParameterName=TK.StringVar()
    BoxType=TK.StringVar()

    BoxName=TK.StringVar()
    NumFrames=TK.IntVar()
    StartFrame=TK.IntVar()
    FrameFreq=TK.IntVar()
    rcutAA=TK.DoubleVar()
    rcutAB=TK.DoubleVar()
    rcutBB=TK.DoubleVar()
    VoroOnOff=TK.IntVar()
    PBCOnOff=TK.IntVar()
    Fc=TK.DoubleVar()
    NumberOfBonds=TK.IntVar()
    CellListOnOff=TK.IntVar()
    writeBonds=TK.IntVar()
    writeAllClusters=TK.IntVar()
    writeRawXmols=TK.IntVar()
    write13Acentres=TK.IntVar()
    writePopPerFrame=TK.IntVar()
    binbonds=TK.DoubleVar()
    writeBondsDistr=TK.IntVar()
    writeBondsDistrPerCluster=TK.IntVar()
    writeBondsDeviations=TK.IntVar()
    writeNeighDistr=TK.IntVar()
    writeDistrCentralBonds=TK.IntVar()
    writeCompositions=TK.IntVar()
    potentialOnOff=TK.IntVar()
    potentialType=TK.IntVar()
    Coslovich=TK.IntVar()
    Dynamics=TK.IntVar()
    writeSubclusters=TK.IntVar()
    TauAlpha=TK.DoubleVar()
    Debug=TK.IntVar()


    # TRAJECTORY
    TrajectoryFrame = TK.LabelFrame(mainframe, text="TRAJECTORY",pady = 2, padx=2,height=80)
    TrajectoryFrame.grid(row=0, column=0,sticky=NSEW)
    
    NameLabel = TK.Label(TrajectoryFrame, text="File name")
    NameLabel.grid(row=1,sticky=NW)

    EName = TK.Entry(TrajectoryFrame, textvariable=name )
    EName.grid(row=1,column=1,sticky=NW)
    name.set("ExampleTrajectory.xyz")

    labelBox = TK.Label(TrajectoryFrame, text="Box")
    labelBox.grid(row=2,column=1,sticky=E)

    labelType = TK.Label(TrajectoryFrame, text="Box Type")
    labelType.grid(row=3,column=0,sticky=W)

    boxOption=TK.OptionMenu(TrajectoryFrame,BoxType,"Cubic", "Non-cubic", "Triclinic")
    boxOption.grid(row=3,column=1, sticky=NSEW)
    BoxType.set("Cubic")


    Nlabel = TK.Label(TrajectoryFrame, text="Number of particles")
    Nlabel.grid(row=4,sticky=NW)

    EN = TK.Entry(TrajectoryFrame, textvariable=N )
    EN.grid(row=4,column=1,sticky=NW)
    N.set(1372)    

    NAlabel = TK.Label(TrajectoryFrame, text="Number of A particles")
    NAlabel.grid(row=5,sticky=NW)

    EA = TK.Entry(TrajectoryFrame, textvariable=NA )
    EA.grid(row=5,column=1,sticky=NW)
    NA.set(686)    

    xsizelabel = TK.Label(TrajectoryFrame, text="Lx")
    xsizelabel.grid(row=6,sticky=NW)

    E1 = TK.Entry(TrajectoryFrame, textvariable=Lx )
    E1.grid(row=6,column=1,sticky=NW)
    Lx.set(10.1917720885902)

    ysizelabel = TK.Label(TrajectoryFrame, text="Ly")
    ysizelabel.grid(row=7,sticky=NW)

    E2 = TK.Entry(TrajectoryFrame, textvariable=Ly )
    E2.grid(row=7,column=1,sticky=NW)
    Ly.set(10.1917720885902)

    zsizelabel = TK.Label(TrajectoryFrame, text="Lz")
    zsizelabel.grid(row=8,sticky=NW)
 
    E3 = TK.Entry(TrajectoryFrame, textvariable=Lz )
    E3.grid(row=8,column=1,sticky=NW)
    Lz.set(10.1917720885902)

    tiltlabel = TK.Label(TrajectoryFrame, text="Tilt")
    tiltlabel.grid(row=9,sticky=NW)

    E4 = TK.Entry(TrajectoryFrame, textvariable=tilt )
    E4.grid(row=9,column=1,sticky=NW)
    tilt.set(0)
    # FRAMES
    labelBox = TK.Label(TrajectoryFrame, text="Frames")
    labelBox.grid(row=10,column=1,sticky=E)

    nFrames = TK.Label(TrajectoryFrame, text="Number of frames")
    nFrames.grid(row=11,sticky=NW)

    E5 = TK.Entry(TrajectoryFrame, textvariable=NumFrames )
    E5.grid(row=11,column=1,sticky=NW)
    NumFrames.set(100)

    startFrame = TK.Label(TrajectoryFrame, text="Start frame")
    startFrame.grid(row=12,sticky=NW)

    E6 = TK.Entry(TrajectoryFrame, textvariable=StartFrame )
    E6.grid(row=12,column=1,sticky=NW)
    StartFrame.set(0)

    freq = TK.Label(TrajectoryFrame, text="Frame sampling frequency")
    freq.grid(row=13,sticky=NW)

    E7 = TK.Entry(TrajectoryFrame, textvariable=FrameFreq )
    E7.grid(row=13,column=1,sticky=NW)
    FrameFreq.set(1)

    tauA = TK.Label(TrajectoryFrame, text="Alpha relaxation (simulation units)")
    tauA.grid(row=14,sticky=NW)

    E8 = TK.Entry(TrajectoryFrame, textvariable=TauAlpha )
    E8.grid(row=14,column=1,sticky=NW)
    TauAlpha.set(1)


    # BONDS
    BondsFrame = TK.LabelFrame(mainframe, text="BONDS",pady = 2, padx=2,height=80)
    BondsFrame.grid(row=0, column=1, sticky=NSEW)

    detect = TK.Label(BondsFrame, text="Detection type ")
    detect.grid(row=1,column=1,sticky=E)

    cVoro = TK.Checkbutton(BondsFrame, text="Voronoi", variable=VoroOnOff)
    cVoro.grid(row=2, column=0, sticky=E)
    VoroOnOff.set(1)
    
    cPBC = TK.Checkbutton(BondsFrame, text="PBC", variable=PBCOnOff)
    cPBC.grid(row=2,column=1, sticky=E)
    PBCOnOff.set(1)
    
    cutBox = TK.Label(BondsFrame, text="Cut Off")
    cutBox.grid(row=3,column=1,sticky=E)

    rcAA = TK.Label(BondsFrame, text="rcutAA")
    rcAA.grid(row=4,sticky=NW)

    rc1 = TK.Entry(BondsFrame, textvariable=rcutAA )
    rc1.grid(row=4,column=1,sticky=NW)
    rcutAA.set(1.4)

    rcAB = TK.Label(BondsFrame, text="rcutAB")
    rcAB.grid(row=5,sticky=NW)

    rc2 = TK.Entry(BondsFrame, textvariable=rcutAB )
    rc2.grid(row=5,column=1,sticky=NW)
    rcutAB.set(1.4)

    rcAB = TK.Label(BondsFrame, text="rcutBB")
    rcAB.grid(row=6,sticky=NW)

    rc2 = TK.Entry(BondsFrame, textvariable=rcutBB )
    rc2.grid(row=6,column=1,sticky=NW)
    rcutBB.set(1.4)

    options = TK.Label(BondsFrame, text="Extra Options")
    options.grid(row=7,column=1,sticky=E)
    maxB = TK.Label(BondsFrame, text="Max # bonds")
    maxB.grid(row=8,sticky=NW)

    maxE = TK.Entry(BondsFrame, textvariable=NumberOfBonds )
    maxE.grid(row=8,column=1,sticky=NW)
    NumberOfBonds.set(30)


    fc = TK.Label(BondsFrame, text="Fc parameter")
    fc.grid(row=9,sticky=NW)

    fcE = TK.Entry(BondsFrame, textvariable=Fc )
    fcE.grid(row=9,column=1,sticky=NW)
    Fc.set(0.82)

    binLabel = TK.Label(BondsFrame, text="binning")
    binLabel.grid(row=10,sticky=NW)
    binE = TK.Entry(BondsFrame, textvariable=binbonds )
    binE.grid(row=10,column=1,sticky=NW)
    binbonds.set(0.02)

    # OUTPUT

    OutputFrame = TK.LabelFrame(mainframe, text="OPTIONAL OUTPUT",pady = 2, padx=2,height=80)
    OutputFrame.grid(row=0, column=2, sticky=NSEW)

    cBonds = TK.Checkbutton(OutputFrame, text="Bonds file", variable=writeBonds)
    cBonds.grid(row=0, column=0, sticky=W)
    writeBonds.set(1)

    cAllClus = TK.Checkbutton(OutputFrame, text="All clusters [large files]", variable=writeAllClusters)
    cAllClus.grid(row=2, sticky=W)
    writeAllClusters.set(0)

    cRaws = TK.Checkbutton(OutputFrame, text="xmol cluster files", variable=writeRawXmols)
    cRaws.grid(row=3,  sticky=W)
    writeRawXmols.set(1)

    cPop = TK.Checkbutton(OutputFrame, text="Population per frame", variable=writePopPerFrame)
    cPop.grid(row=4,  sticky=W)
    writePopPerFrame.set(0)

    cCen = TK.Checkbutton(OutputFrame, text="13A centres", variable=write13Acentres)
    cCen.grid(row=5,  sticky=W)
    write13Acentres.set(0)

    cBL = TK.Checkbutton(OutputFrame, text="Bond length distributions", variable=writeBondsDistr)
    cBL.grid(row=6,  sticky=W)
    writeBondsDistr.set(0)

    # cBLT = TK.Checkbutton(OutputFrame, text="Bond length distributions per cluster", variable=writeBondsDistrPerCluster)
    # cBLT.grid(row=7,  sticky=W)
    # writeBondsDistrPerCluster.set(0)

    cnb = TK.Checkbutton(OutputFrame, text="Coordination distribution", variable=writeNeighDistr)
    cnb.grid(row=7,  sticky=W)
    writeNeighDistr.set(0)

    cnb = TK.Checkbutton(OutputFrame, text="Cluster compositions ", variable=writeCompositions)
    cnb.grid(row=8,  sticky=W)
    writeCompositions.set(0)

    cCoslo = TK.Checkbutton(OutputFrame, text="Coslovich Voronoi-face analysis", variable=Coslovich)
    cCoslo.grid(row=9,  sticky=W)

    # csub = TK.Checkbutton(OutputFrame, text="Sub-clusters", variable=writeSubclusters)
    # csub.grid(row=11,  sticky=W)
    # writeSubclusters.set(0)

    status=TK.StringVar()

    statusLabel = TK.Label(mainframe, textvariable=status)
    statusLabel.grid(row=12, column=0, sticky=NSEW, columnspan=2)
    status.set("Ready")
    statusLabel.configure(bg='black', fg='white')

    def writescripts():
        try:
            rho=N.get()/(Lx.get()*Ly.get()*Lz.get())
            boxfile="box.txt"
            fhd=open("d0_"+boxfile,'w')
            fhd.write("#iter LX LY LZ tilt\n%d %lg %lg %lg %g\n"% (int(StartFrame.get()), Lx.get(),Ly.get(),Lz.get(), tilt.get()))
            fhd.close()

            file1=open("d0_init.params","w")
            file1.write("d0_init.params\n"+name.get()+"    xyz file\nno   ir file\nno     v file\n"+str(N.get())+"  N\n"+str(NA.get())+"  NA\n"+str(rho)+" rho\n"+str(StartFrame.get())+"  TSTART\n"+str(FrameFreq.get())+"  INITIAL_FRAMETSTEP\n"+str(StartFrame.get()+NumFrames.get())+"  TEND\n"+str(NumFrames.get())+" TOTALFRAMES\n")
            file1.close()

            file2=open("inputparameters.in","w")
            BOXTYPE=0
            if(BoxType.get()=='Non-cubic'):
                BOXTYPE=1
            elif(BoxType.get()=='Triclinic'):
                BOXTYPE=3
            print "You have selected a "+BoxType.get()+" box"
            file2.write("""init.params    // name of parameters file for xmol trajectory, minus d[rank]_ prefix
"""+str(BOXTYPE)+"""    // ISNOTCUBIC - 0 if cubic NVT, 1 if system non-cubic NVT, 2 if system is NPT, 3 triclinic with tilt   INTEGER
"""+boxfile+"""    // name of parameters file for box size data, minus "d[rank]_" prefix
"""+str(NumFrames.get())+"""    // FRAMES - frames to read from input xmol file                                         INTEGER
"""+str(StartFrame.get())+"""    // STARTFROM - start reading from this frame in the xmol file                               INTEGER
"""+str(FrameFreq.get())+"""    // SAMPLEFREQ - frequency at which to take frames from the xmol file                            INTEGER
"""+str(rcutAA.get())+"""    // rcutAA - specie-specie bond lengths only work with simple                               DOUBLE
"""+str(rcutAB.get())+"""    // rcutAB - specie-specie bond lengths only work with simple    Paddy scaled to be 1.4 sigmaAB scaled                           DOUBLE
"""+str(rcutBB.get())+"""    // rcutBB - specie-specie bond lengths only work with simple    Paddy scaled to be 1.4 sigmaBB scaled                           DOUBLE
"""+str(VoroOnOff.get())+"""    // 0 simple bond length, 1 Voronoi bond detection                                       BINARY INTEGER
1    //0 off, 1 on, Use period boundary conditions (PBCs), 2 Use Lees-Edwards BCs - only with simple bond               BINARY INTEGER
"""+str(Fc.get())+"""    // Modified Voronoi Fc parameter                                                    DOUBLE IN [0,1]
"""+str(NumberOfBonds.get())+"""    // nB - max number of bonds to one particle                                         INTEGER
0    // use Cell List to calculate bond network (and potential if used as well)                              BINARY INTEGER
"""+str(writeBonds.get())+"""    // write out bonds file                                                         BINARY INTEGER
"""+str(writeAllClusters.get())+"""    // write clusts_** files containing all clusters - USES LOTS OF HDD SPACE                       BINARY INTEGER
"""+str(writeRawXmols.get())+"""    // write raw_** xmol cluster files                                          BINARY INTEGER
"""+str(write13Acentres.get())+"""    // write centres of 13A                                                 BINARY INTEGER
"""+str(writePopPerFrame.get())+"""    // write particle fraction of each cluster per frame (pop_per_frame)                            BINARY INTEGER
"""+str(binbonds.get())+"""    // bin width for bond length distributions                                              DOUBLE
"""+str(writeBondsDistr.get())+"""    // write bond length distributions                                                  BINARY INTEGER
"""+str(writeBondsDistr.get())+"""    // write bond length distributions for each cluster type                                    BINARY INTEGER
"""+str(0)+"""    // write bond length deviations from ground.state.bondlengths.dat for each cluster type             BINARY INTEGER
"""+str(writeNeighDistr.get())+"""    // write number of neighbour distributions                                              BINARY INTEGER
"""+str(writeDistrCentralBonds.get())+"""    // write distributions for the number of particles bonded to the centre of each cluster             BINARY INTEGER
"""+str(writeCompositions.get())+"""    / write compositions of each cluster in terms of A and B species                               BINARY INTEGER
"""+str(0)+"""    / do potential energy calculations                                                 BINARY INTEGER
"""+str(0)+"""    // 0 BLJ, 1 SFBLJ, 2 MorYuk: polydisp morse+yukawa, 3 not used, 4 IPL, 5 BLJ_WCAs, 6 SFIPL, 7 CRVT  INTEGER
"""+str(Coslovich.get())+"""    // do Coslovich-style Voronoi faces analysis                                            BINARY INTEGER
"""+str(Dynamics.get())+"""    // do Dynamics Analysis - choose which clusters and set memory sizes in static.memsize.dat          BINARY INTEGER
"""+str(0)+"""    // write subclusters of each cluster, if dynamics also done on required subcluster                  BINARY INTEGER
"""+str(TauAlpha.get())+"""    // alpha relaxtion time (in simulation time units)                                          DOUBLE
"""+str(0)+"""    // printing running (per frame) debug information                                       BINARY INTEGER

Potential parameters are in potentialparams.in\n""")
            file2.close()
            status.set("!!! SUCCESS !!!")
            statusLabel.configure(bg='green', fg='white')
          
        except Exception, e:
            status.set("!!! Error : "+str(e)+" !!!")
            statusLabel.configure(bg='red', fg='white')
    writeButton=TK.Button(mainframe,text="Write parameter files",width=34,command=lambda:writescripts())
    writeButton.grid(row=11, column=2,sticky=S)
    print ""
    def close_window (): 
        root.destroy()
    close=TK.Button(mainframe,text="Quit",width=34,command=close_window)
    close.grid(row=12, column=2,sticky=S)

    root.mainloop()


