import vtk
from array import *
import numpy
import os


# todo range the folder to get the file name

gridnum = 15

massR = 4

massOrigin = [6,0,6]

initVlue = 1.5

targetValue = 7.5

# detect the value at iteration timestep 41
# interested event
# i 6 7 8 9
# j 0 1 2 3
# k 6 7 8 9

rootDir = "./image"

# refer to https://xrunhprof.wordpress.com/2012/02/06/array-exchange-between-vtk-and-python/
def vtk_to_array(vtk_array):
    at = vtk_array.GetDataType()
    if at == 11:
        #vtkDoubleArray
        pt='d'
    elif at == 12:
        #vtkIdTypeArray
        pt='l'
    #this is slow. numpy.zeros would be faster.
    r = array(pt, [0]*vtk_array.GetSize())
    vtk_array.ExportToVoidPointer(r)
    return r


# draw graph
def drawGraph(filePath):
    #TODO call the paraview to draw graph
    print ("draw graph for")
    print (filePath)
    

for dirName, subdirList, fileList in os.walk(rootDir):
    print('Found directory: %s' % dirName)
    for fname in fileList:
        filePath = rootDir+"/"+fname
        #print filePath

        # Read the file (to test that it was written correctly)
        reader = vtk.vtkXMLImageDataReader()
        reader.SetFileName(filePath)
        reader.Update()

        obj = reader.GetOutput().GetCellData()

        # use dir(obj) to get all the function of the class

        # print dir(obj)

        vtkarray = obj.GetArray("pressure")

        parray = vtk_to_array(vtkarray)


        #print parray
        nparray  =  numpy.asarray(parray).reshape(gridnum, gridnum, gridnum, order='F')

        #print nparray[1][1]

        # detect if all the mass info is targeted one

        ifTargetEventHappen = True

        #if (fname == "image41.vti"):
        #    print nparray

        for i in range (massOrigin[0],massOrigin[0]+massR):
            for j in range (massOrigin[1],massOrigin[1]+massR):
                for k in range (massOrigin[2],massOrigin[2]+massR):
                    #print "index i j k (%d %d %d)" % (i,j,k)
                    #print  nparray[i][j][k]
                    #print "index i j k (%d %d %d)" % (i,j,k)
                    #print nparray[i][j][k]
                    if (nparray[i][j][k]!=targetValue):
                        ifTargetEventHappen = False
                        break
        if (ifTargetEventHappen == True):
            print (filePath)

            drawGraph(filePath)

            exit(0)

        # draw the graph

# if the interested things happen
# output the timestep info
# and send the command to draw out the picture



'''
# Convert the image to a polydata
imageDataGeometryFilter = vtk.vtkImageDataGeometryFilter()
imageDataGeometryFilter.SetInputConnection(reader.GetOutputPort())
imageDataGeometryFilter.Update()

mapper = vtk.vtkPolyDataMapper()
mapper.SetInputConnection(imageDataGeometryFilter.GetOutputPort())

actor = vtk.vtkActor()
actor.SetMapper(mapper)
actor.GetProperty().SetPointSize(3)

# Setup rendering
renderer = vtk.vtkRenderer()
renderer.AddActor(actor)
renderer.SetBackground(1, 1, 1)
renderer.ResetCamera()

renderWindow = vtk.vtkRenderWindow()
renderWindow.AddRenderer(renderer)

renderWindowInteractor = vtk.vtkRenderWindowInteractor()

renderWindowInteractor.SetRenderWindow(renderWindow)
renderWindowInteractor.Initialize()
renderWindowInteractor.Start()
'''