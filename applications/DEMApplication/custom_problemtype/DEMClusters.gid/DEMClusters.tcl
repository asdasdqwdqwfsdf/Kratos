
### Brief methodology to generate the cluster file ###
# 1. extract from mesh: OBJ from surface mesh, MSH from the tetrahedra mesh
# 2. Generate SPH via sphereTree external executable passing OBJ file as argument (current issue accessing gid values)
# 3. Clean the SPH file
# 4. Generate Cluster.clu via mesh_to_cluster_converter.cpp passing SPH + MSH as arguments
#    modified and precompiled executable will be required to do this


# current order and links
# 1AfterMeshGeneration
# -ExtractSurfaceTriangles - aux run on calculate, folder and file save

# 2OnCalculateExecution
# -Generate_OBJFile

# 3GenerateSPHFileFromOBJFile
# -call_SphereTree

# 4Generate.ClusterFile



##  --------------------------------------------------------------------------------------------------------------------------------------------------
##  Current Issues ## 

## GenerateClusterFile works fine in the folder but shows segmentation fault when launching from gid

##- spheretree throws error if algorithm parameters are not quite good. example: small geom with high numsamples 
##- even if spheretree works as expected, it throw an error when finalizing. kike: child process exited abnormally



##  --------------------------------------------------------------------------------------------------------------------------------------------------
##  Fixed Issues ## 

##- Are ALL calculated Vertex normals correct? verified manually with testcubev4
##- Sphere tree paths with spaces
##- Dependencies in prb.  each line for an algorithm, one parenthesis
##- verify that your geometry have all the normals coherent. (should be as align normals has been added before meshing)
##- add some info on the help based on the manual
##- Calling external precompiled cpp, add criteria for negative radius when deleting line in sph
##- plan on msh to clu functionaly design of precompiled executable. args, paths and location, 
##-     define arguments path and call exe from inside exec folder.
##- add dummy .bat to avoid error showing both unix.bat and win.bat
##- cluster visualizaton in GID pre
##- add export gidmesh as generic.msh on calculate
##  --------------------------------------------------------------------------------------------------------------------------------------------------




## GiD events --------------------------------------------------------------------------------------------------------------------------------------------------


proc InitGIDProject { dir } {

    # Create buttons to insert algorithm values
    if { [GidUtils::IsTkDisabled] eq 0} {
        GiDMenu::Create "Sphere Cluster Creation" PRE
        #GiDMenu::InsertOption "Sphere Cluster Creation" [list "SphereTree"] 0 PRE "GidOpenConditions \"SphereTree\"" "" ""
        GiDMenu::InsertOption "Sphere Cluster Creation" [list "Define Options"] 0 PRE "GidOpenProblemData" "" ""
        GiDMenu::InsertOption "Sphere Cluster Creation" [list "GenerateSPH" ] 1 PRE [list GenerateSPHFileFromOBJFile] "" ""
        GiDMenu::InsertOption "Sphere Cluster Creation" [list "GenerateCLU" ] 2 PRE [list GenerateClusterFile] "" ""
        GiDMenu::InsertOption "Sphere Cluster Creation" [list "Visualize cluster" ] 3 PRE [list ReadClusterFile] "" ""
        GiDMenu::UpdateMenus
    }

    # Load the application scripts
    set scripts_dir [file join $dir .. .. ]
    # set tcl_filename [file join $scripts_dir scripts initptype.tcl]
    # if {[catch {source $tcl_filename} msg]} {
    #     WarnWinText $msg
    #     return 1
    # }

    # Save ProblemTypePath
    set ::DEMClusters::ProblemTypePath $dir
}


proc AfterReadGIDProject { filename } {

    # Save ProblemPath
    set projectpath $filename
    append projectpath .gid
    set ::DEMClusters::ProblemPath $projectpath

    # Save ProblemName
    if {[regexp -all {\\} $filename] > 0} {
        # Windows
        regsub -all {\\} $filename { } filename
    } else {
        # Unix
        regsub -all {/} $filename { } filename
    }
    set filename [lreplace $filename 0 [expr { [llength $filename]-2 }]]
    set ::DEMClusters::ProblemName $filename
}


#proc BeforeMeshGeneration {elementsize}
proc GiD_Event_BeforeMeshGeneration {elementsize} {

    # Align the normal
    AlignSurfNormals Outwards
    GiD_MeshData mesh_criteria to_be_meshed 2 surfaces [GiD_Geometry list surface]
}

proc AfterMeshGeneration {fail} {

}


proc BeforeRunCalculation { batfilename basename dir problemtypedir gidexe args } {
    source [file join $problemtypedir OBJFile.tcl]
    set OBJOutput [GenerateOBJFile $basename $dir $problemtypedir]

    set modelname [GiD_Info Project ModelName]
    set export_msh [file join ${modelname}.gid generic.msh]
    GiD_Process Mescape Files WriteMesh $export_msh
}


proc GenerateSPHFileFromOBJFile { } {
    call_SphereTree
}


namespace eval DEMClusters {
    variable ProblemName ""
    variable ProblemPath ""
    variable ProblemTypePath ""
}


proc call_SphereTree { } {

    set Algorithm [GiD_AccessValue get gendata Algorithm]
    if {$Algorithm == "MakeTreeMedial"} {
        ::DEMClusters::call_TreeMedial
    } elseif {$Algorithm == "MakeTreeGrid"} {
        ::DEMClusters::call_makeTreeGrid
    } elseif {$Algorithm == "MakeTreeSpawn"} {
        ::DEMClusters::call_makeTreeSpawn
    } elseif {$Algorithm == "MakeTreeOctree"} {
        ::DEMClusters::call_makeTreeOctree        
    } elseif {$Algorithm == "MakeTreeHubbard"} {
        ::DEMClusters::call_makeTreeHubbard
    } else {
        W "Select a valid algorithm"
    }

    # set Young_Modulus [GiD_AccessValue get condition Body_Part Young_Modulus]
    
}

proc DEMClusters::call_TreeMedial { } {

    set Algorithm [GiD_AccessValue get gendata Algorithm]
    set branch [GiD_AccessValue get gendata branch]
    set depth [GiD_AccessValue get gendata depth]
    set testerLevels [GiD_AccessValue get gendata testerLevels]
    set numCover [GiD_AccessValue get gendata numCover]
    set minCover [GiD_AccessValue get gendata minCover]
    set initSpheres [GiD_AccessValue get gendata initSpheres]
    set minSpheres [GiD_AccessValue get gendata minSpheres]
    set erFact [GiD_AccessValue get gendata erFact]
    set numSamples [GiD_AccessValue get gendata numSamples]
    set minSamples [GiD_AccessValue get gendata minSamples]
    set genericOBJFilename [file join $::DEMClusters::ProblemPath generic.obj]
    set genericOBJFilename "\"$genericOBJFilename\""

    #set filename_obj $::DEMClusters::ProblemName ## custom names
    #append filename_obj .obj

    set argv "-depth $depth -branch $branch -numCover $numCover -minCover $minCover -initSpheres $initSpheres -minSpheres $minSpheres -erFact $erFact -testerLevels $testerLevels -verify -nopause -eval -expand -merge -burst -optimise balance -balExcess 0.001 -maxOptLevel 100 $genericOBJFilename"
    set program [file join $::DEMClusters::ProblemTypePath exec $Algorithm]
    exec $program {*}$argv

    # MakeTreeMedial -depth 3 -branch 8 -numCover 10000 -minCover 5 -initSpheres 1000 -minSpheres 200 -erFact 2 -testerLevels 2 -verify -nopause -eval -expand -merge -burst -optimise balance -balExcess 0.001 -maxOptLevel 100 generic.obj
    #set program [lindex $argv 0]
    # set program [file join $::DEMClusters::ProblemTypePath exec MakeTreeMedial.exe]
    # set arguments [lrange $argv 1 end]
    # exec $program {*}$arguments
   
    # TreeMedial ValidArgs:
    # -depth              Depth of the sphere-tree
    # -branch             Branching factor of sphere-tree
    # -numCover           Number of sample points to cover object with
    # -minCover           Minimum number of sample points per triangle
    # -initSpheres        Initial number of spheres in medial axis approx.
    # -minSpheres         Minimum number of spheres to create for each sub
    #                         region of the medial axis approximation.
    # -erFact             Amount by which to reduce the error when refining
    #                         the medial axis approximation.
    # -testerLevels       Controls the number of points to use to represent a
    #                         sphere when evaluating fit.  Use -1 for CONVEX
    #                         objects, 1 will generate 42 points and 2 will
    #                         generate 168 points.
    # -optimise           Which optimisation algorithm to use, SIMPLEX just
    #                         rearranges the spheres to try improve fit, BALANCE
    #                         tries to throw away spheres that don't improve the
    #                         approximation.
    # -maxOptLevel        Maximum level of the sphere-tree to apply the optimiser.
    #                         0 does first set only - i.e. children of level 0.
    # -balExcess          The amount of extra error the BALANCE algorithm is
    #                         allowed to introduce when throwing away error,
    #                         e.g. 0.05 allows a 5 percent increase in the error.
    # -verify             Verify the model is suitable for use
    # -nopause            Don't pause when processing, i.e. batch mode
    # -eval               Evaluate the fit of the sphere-tree and append the info
    #                         to the end of the output file.
    # -merge              Try the MERGE, BURST and EXPAND algorithms.  You can
    # -burst              specify any number of these that you wish.
    # -expand

}

proc DEMClusters::call_makeTreeGrid { } {

    set Algorithm [GiD_AccessValue get gendata Algorithm]
    set depth [GiD_AccessValue get gendata depth]
    set numCover [GiD_AccessValue get gendata numCover]
    set minCover [GiD_AccessValue get gendata minCover]
    set testerLevels [GiD_AccessValue get gendata testerLevels]
    set genericOBJFilename [file join $::DEMClusters::ProblemPath generic.obj]
    set genericOBJFilename "\"$genericOBJFilename\""

    set argv "-depth $depth -branch $branch -numCover $numCover -minCover $minCover -testerLevels $testerLevels -verify -nopause -eval $genericOBJFilename"
    set program [file join $::DEMClusters::ProblemTypePath exec $Algorithm]
    exec $program {*}$argv

    # makeTreeGrid ValidArgs:
    # -depth              Depth of the sphere-tree
    # -numCover           Number of sample points to cover object with
    # -minCover           Minimum number of sample points per triangle
    # -testerLevels       Controls the number of points to use to represent a
    #                         sphere when evaluating fit.  Use -1 for CONVEX
    #                         objects, 1 will generate 42 points and 2 will
    #                         generate 168 points.
    # -verify             Verify the model is suitable for use
    # -nopause            Don't pause when processing, i.e. batch mode
    # -eval               Evaluate the fit of the sphere-tree and append the info
    #                         to the end of the output file.
}

proc DEMClusters::call_makeTreeSpawn { } {

    set Algorithm [GiD_AccessValue get gendata Algorithm]
    set depth [GiD_AccessValue get gendata depth]
    set numCover [GiD_AccessValue get gendata numCover]
    set minCover [GiD_AccessValue get gendata minCover]
    set testerLevels [GiD_AccessValue get gendata testerLevels]
    set genericOBJFilename [file join $::DEMClusters::ProblemPath generic.obj]
    set genericOBJFilename "\"$genericOBJFilename\""

    set argv "-depth $depth -branch $branch -numCover $numCover -minCover $minCover -testerLevels $testerLevels -verify -nopause -eval $genericOBJFilename"
    set program [file join $::DEMClusters::ProblemTypePath exec $Algorithm]
    exec $program {*}$argv

    # makeTreeSpawn ValidArgs:
    # -depth              Depth of the sphere-tree
    # -numCover           Number of sample points to cover object with
    # -minCover           Minimum number of sample points per triangle
    # -testerLevels       Controls the number of points to use to represent a
    #                         sphere when evaluating fit.  Use -1 for CONVEX
    #                         objects, 1 will generate 42 points and 2 will
    #                         generate 168 points.
    # -verify             Verify the model is suitable for use
    # -nopause            Don't pause when processing, i.e. batch mode
    # -eval               Evaluate the fit of the sphere-tree and append the info
    #                         to the end of the output file.
}



proc DEMClusters::call_makeTreeHubbard { } {

    set Algorithm [GiD_AccessValue get gendata Algorithm]
    set branch [GiD_AccessValue get gendata branch]
    set depth [GiD_AccessValue get gendata depth]
    set numSamples [GiD_AccessValue get gendata numSamples]
    set minSamples [GiD_AccessValue get gendata minSamples]
    set genericOBJFilename [file join $::DEMClusters::ProblemPath generic.obj]
    #set genericOBJFilename "\"$genericOBJFilename\""

    set modelname [GiD_Info Project ModelName]
    set genericOBJFilename [file join ${modelname}.gid generic.obj]

    set argv "-depth $depth -branch $branch -numSamples $numSamples -minSamples $minSamples -nopause $genericOBJFilename"
    set program [file join $::DEMClusters::ProblemTypePath exec $Algorithm]
    exec $program {*}$argv

    # makeTreeHubbard ValidArgs:
    # -depth              Depth of the sphere-tree
    # -branch             Branching factor of sphere-tree
    # -numSamples         Number of sample points to cover object with
    # -minSamples         Minimum number of sample points per triangle
    # makeTreeHubbard  -branch 8 -depth 3 -numSamples 500 -minSamples 1 -nopause generic.obj
}

proc DEMClusters::call_makeTreeOctree { } {

    set Algorithm [GiD_AccessValue get gendata Algorithm]
    set depth [GiD_AccessValue get gendata depth]
    set genericOBJFilename [file join $::DEMClusters::ProblemPath generic.obj]
    set genericOBJFilename "\"$genericOBJFilename\""

    set argv "-depth $depth -nopause $genericOBJFilename"
    set program [file join $::DEMClusters::ProblemTypePath exec $Algorithm]
    exec $program {*}$argv

    # makeTreeOctree ValidArgs:
    # -depth              Depth of the sphere-tree
    # -nopause            Don't pause when processing, i.e. batch mode
}

proc GenerateClusterFile { } {

    set Algorithm [GiD_AccessValue get gendata Algorithm]
    if {$Algorithm == "MakeTreeMedial"} {
        set genericSPHFilename [file join $::DEMClusters::ProblemPath generic-medial.sph]
        set genericSPHFilename "\"$genericSPHFilename\""
    } elseif {$Algorithm == "MakeTreeGrid"} {
        set genericSPHFilename [file join $::DEMClusters::ProblemPath generic-grid.sph]
        set genericSPHFilename "\"$genericSPHFilename\""
    } elseif {$Algorithm == "MakeTreeSpawn"} {
        set genericSPHFilename [file join $::DEMClusters::ProblemPath generic-spawn.sph]
        set genericSPHFilename "\"$genericSPHFilename\""
    } elseif {$Algorithm == "MakeTreeOctree"} {    
        set genericSPHFilename [file join $::DEMClusters::ProblemPath generic-octree.sph]
        set genericSPHFilename "\"$genericSPHFilename\""
    } elseif {$Algorithm == "MakeTreeHubbard"} {
        #set genericSPHFilename generic-hubbard.sph
        set genericSPHFilename [file join $::DEMClusters::ProblemPath generic-hubbard.sph]
        set genericSPHFilename "\"$genericSPHFilename\""
    } else {
        W "Select a valid algorithm"
    }
     
    set cluster_exec create_cluster.exe
    set genericMSHFilename [file join $::DEMClusters::ProblemPath generic.msh]
    set genericMSHFilename "\"$genericMSHFilename\""
    set argv_number 2

    set argv "$argv_number $genericMSHFilename $genericSPHFilename"
    set program [file join $::DEMClusters::ProblemTypePath exec $cluster_exec]
    #set program "\"$program\""
    W $argv
    W $program

    #set fid [open "| $program $argv_number $genericMSHFilename $genericSPHFilename" r+]
    #close $fid
    #exec $program $argv_number $genericMSHFilename $genericSPHFilename
    #exec {*}[auto_execok start] $program $argv_number $genericMSHFilename $genericSPHFilename
    exec $program {*}$argv
}


proc AlignSurfNormals {direction} {
    # ABSTRACT: Makes all of boundary surfaces' normals point inwards or outwards
    # Arguments
    # direction => Direction option ["Inwards"|"Outwards"]
    # Note: This procedure in the same used in the fluid_only problem type

    switch $direction {
        Inwards {
            set wrong_way "DIFF1ST"
        }
        Outwards {
            set wrong_way "SAME1ST"
        }
        default {puts "Unknown Direction, surface normals not aligned"}
    }
    set volumelist [GiD_Geometry list volume 1:]
    set surfacelist [list]

    # For each volume, we look for face surfaces with oriented in the wrong direction
    foreach volume $volumelist {
        set volumeinfo [GiD_Info list_entities volumes $volume]
        set numpos [lsearch $volumeinfo "NumSurfaces:"]
        set numsurf [lindex $volumeinfo [expr {$numpos +1 }]]
        for {set i 0} {$i < $numsurf} {incr i} {
            set orient [lindex $volumeinfo [expr {$numpos+5+4*$i}]]
            if {[string compare $orient $wrong_way]==0} {
                # If the normal is pointing in the wrong direction,
                # Check if it's a contour surface
                set surfnum [lindex $volumeinfo [expr {$numpos+3+4*$i}]]
                set surfinfo [GiD_Info list_entities surfaces $surfnum]
                set higherentities [lindex $surfinfo 4]
                if {$higherentities==1} {
                    lappend surfacelist $surfnum
                }
            }
        }
    }

    if {[llength $surfacelist]} {
        # If its in the contour, switch its normal
        eval GiD_Process Mescape Utilities SwapNormals Surfaces Select $surfacelist
    }
}



proc ReadClusterFile { } {

    set modelname [GiD_Info Project ModelName]
    set clustername [file join ${modelname}.gid generic_cluster.clu]
    set FileVar [open $clustername "r+"]
    set file_data [read $FileVar]
    lreplace $file_data end end
    close $FileVar

    #  Process data file
    set data [split $file_data "\n"]
    set data [lreplace $data end-14 end] 
    foreach line $data {
        if {[llength $line] >3} {
            W $line
            GiD_Process Mescape Geometry Create Object Sphere [lrange $line 0 0] [lrange $line 1 1] [lrange $line 2 2] [lrange $line 3 3] escape escape
        } 
    }
}






# GiD_Mesh get element <num|from_face|from_edge> ?face|face_linear|num_faces|edge_linear|num_edges|normal|tangent|center|connectivities ?<face_id>|<edge_id>??

# If from_face is specified then the command has this syntax

# GiD_Mesh get element from_face <face_nodes> ?-ordered?

# it find and return the list of element ids that have a face with these nodes

# <face_nodes> is the list of integer ids of the face nodes {<face_node_1> ... <face_node_n>} (only corner lineal nodes must be specified in the list)

# if -ordered is specified then only faces with the same orientation of the nodes will be taken into account (else the ordenation of the face nodes doesn't matter)

# If from_edge is specified then the command has this syntax

# GiD_Mesh get element from_edge <edge_nodes>

# it find and return the list of element ids that have an edge with these nodes

# <edge_nodes> is the list of integer ids of the edge nodes {<edge_node_1> <edge_node_2>} (only corner lineal nodes must be specified in the list)