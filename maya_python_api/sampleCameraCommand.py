# sampleCameraCommand.py

import sys
import maya.OpenMayaMPx as OpenMayaMPx
import maya.OpenMaya as OpenMaya
import maya.OpenMayaUI as OpenMayaUI

kPluginCmdName = 'myCameraCommand'

##########################################################
# Plug-in
##########################################################


class MyCameraCommand(OpenMayaMPx.MPxCommand):

    def __init__(self):
        ''' Constructor. '''
        OpenMayaMPx.MPxCommand.__init__(self)

    def doIt(self, args):
        ''' doIt() is called once when the command is first executed. '''

        # This MDagModifier object will allow us to undo and redo the creation
        # of DAG nodes in our command.
        self.dagModifier = OpenMaya.MDagModifier()

        # Create the camera transform node.
        self.cameraTransformObj = self.dagModifier.createNode('transform')
        self.dagModifier.renameNode(
            self.cameraTransformObj, 'myCameraTransform')

        # Create the camera shape node as a child of the camera transform node.
        self.cameraShapeObj = self.dagModifier.createNode(
            'camera', self.cameraTransformObj)
        self.dagModifier.renameNode(self.cameraShapeObj, 'myCameraShape')

        # Call self.redoIt() to perform the command's actual work. This function call flow
        # is useful for code re-use.
        self.redoIt()

    def redoIt(self):
        ''' redoIt() is called every time the instance of this command is re-done from
        the undo queue. '''

        # Perform the operations enqueued within our reference to MDagModifier. This effectively
        # creates the DAG nodes specified using self.dagModifier.createNode().
        self.dagModifier.doIt()

        # Set the translation value of the camera's transform node.
        transformFn = OpenMaya.MFnTransform(self.cameraTransformObj)
        transformFn.setTranslation(
            OpenMaya.MVector(0, 5, 30), OpenMaya.MSpace.kTransform)

        # Store the previous camera before we switch to the camera created within this command.
        # In undo() we will revert to this previous camera.
        self.previousCamera = OpenMaya.MDagPath()
        currentView = OpenMayaUI.M3dView.active3dView()
        # self.previousCamera is now populated with the current camera before
        # we switch.
        currentView.getCamera(self.previousCamera)

        # Get the DAG path of our camera shape node.
        cameraDagPath = OpenMaya.MDagPath()
        dagNodeFn = OpenMaya.MFnDagNode(self.cameraShapeObj)
        dagNodeFn.getPath(cameraDagPath)

        # Set the camera view to the one we switched
        currentView.setCamera(cameraDagPath)

    def undoIt(self):
        ''' undoIt() is called every time the instance of this command is undone. '''

        # Switch back to the previous camera. We do not have to reverse the translation of
        # self.cameraTransformObj because it will be excluded from the DAG once
        # self.dagModifier.undoIt() is called below.
        currentView = OpenMayaUI.M3dView.active3dView()
        currentView.setCamera(self.previousCamera)

        # This call to MDagModifier.undoIt() undoes all the operations within
        # the MDagModifier.
        self.dagModifier.undoIt()

    def isUndoable(self):
        ''' This function indicates whether or not the command is undoable. If the
        command is undoable, each executed instance of that command is added to the
        undo queue. '''

        # We must return True to specify that this command is undoable.
        return True


##########################################################
# Plug-in initialization.
##########################################################
def cmdCreator():
    ''' Create an instance of our command. '''
    return OpenMayaMPx.asMPxPtr(MyCameraCommand())


def initializePlugin(mobject):
    ''' Initialize the plug-in when Maya loads it. '''
    mplugin = OpenMayaMPx.MFnPlugin(mobject)
    try:
        mplugin.registerCommand(kPluginCmdName, cmdCreator)
    except:
        sys.stderr.write('Failed to register command: ' + kPluginCmdName)


def uninitializePlugin(mobject):
    ''' Uninitialize the plug-in when Maya un-loads it. '''
    mplugin = OpenMayaMPx.MFnPlugin(mobject)
    try:
        mplugin.deregisterCommand(kPluginCmdName)
    except:
        sys.stderr.write('Failed to unregister command: ' + kPluginCmdName)

##########################################################
# Sample usage.
##########################################################
''' 
# Copy the following lines and run them in Maya's Python Script Editor:

import maya.cmds as cmds
cmds.loadPlugin( 'sampleCameraCommand.py' )
cmds.myCameraCommand()
cmds.undo()
cmds.redo()

'''
