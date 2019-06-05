/*
	 ------------------------------------------------------------------

	 This file is part of the Open Ephys GUI
	 Copyright (C) 2014 Open Ephys

	 ------------------------------------------------------------------

	 This program is free software: you can redistribute it and/or modify
	 it under the terms of the GNU General Public License as published by
	 the Free Software Foundation, either version 3 of the License, or
	 (at your option) any later version.

	 This program is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	 GNU General Public License for more details.

	 You should have received a copy of the GNU General Public License
	 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include "MainWindow.h"
#include "UI/UIComponent.h"
#include "UI/EditorViewport.h"
#include <stdio.h>
#include <future>
//-----------------------------------------------------------------------

static inline File getSavedStateDirectory() {
#if defined(__APPLE__)
    File dir = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Application Support/open-ephys");
    if (!dir.isDirectory()) {
        dir.createDirectory();
    }
    return std::move(dir);
#else
    return File::getSpecialLocation(File::currentExecutableFile).getParentDirectory();
#endif
}

MainWindow::MainWindow(const File& fileToLoad)
: DocumentWindow(JUCEApplication::getInstance()->getApplicationName(),
		Colour(Colours::black),
		DocumentWindow::allButtons)
{

	setResizable(true,      // isResizable
			false);   // useBottomCornerRisizer -- doesn't work very well

	shouldReloadOnStartup = false;

	// Create ProcessorGraph and AudioComponent, and connect them.
	// Callbacks will be set by the play button in the control panel

	processorGraph = new ProcessorGraph();
	std::cout << std::endl;
	std::cout << "Created processor graph." << std::endl;
	std::cout << std::endl;

	audioComponent = new AudioComponent();
	std::cout << "Created audio component." << std::endl;

	audioComponent->connectToProcessorGraph(processorGraph);

	setContentOwned(new UIComponent(this, processorGraph, audioComponent), true);

	UIComponent* ui = (UIComponent*) getContentComponent();

	commandManager.registerAllCommandsForTarget(ui);
	commandManager.registerAllCommandsForTarget(JUCEApplication::getInstance());

	ui->setApplicationCommandManagerToWatch(&commandManager);

	addKeyListener(commandManager.getKeyMappings());

	loadWindowBounds();
	setUsingNativeTitleBar(true);
	Component::addToDesktop(getDesktopWindowStyleFlags());  // prevents the maximize
	// button from randomly disappearing
	setVisible(true);

	// Constraining the window's size doesn't seem to work:
	setResizeLimits(500, 500, 10000, 10000);

    if (!fileToLoad.getFullPathName().isEmpty())
    {
        ui->getEditorViewport()->loadState(fileToLoad);
    }
	else if (shouldReloadOnStartup)
	{
		File file = getSavedStateDirectory().getChildFile("lastConfig.xml");
		uiLoader = new UIBackgroundLoader(ui, file);
		uiLoader->startThread();
		//ui->getEditorViewport()->loadState(file);
	}

}

MainWindow::~MainWindow()
{

	if (audioComponent->callbacksAreActive())
	{
		audioComponent->endCallbacks();
		processorGraph->disableProcessors();
	}

	saveWindowBounds();

	audioComponent->disconnectProcessorGraph();
	UIComponent* ui = (UIComponent*) getContentComponent();
	ui->disableDataViewport();

	File file = getSavedStateDirectory().getChildFile("lastConfig.xml");
	ui->getEditorViewport()->saveState(file);

	setMenuBar(0);

#if JUCE_MAC
	MenuBarModel::setMacMainMenu(0);
#endif

}

void MainWindow::closeButtonPressed()
{
	if (audioComponent->callbacksAreActive())
	{
		audioComponent->endCallbacks();
	}

	processorGraph->disableProcessors();

	JUCEApplication::getInstance()->systemRequestedQuit();

}

void MainWindow::saveWindowBounds()
{
	std::cout << std::endl;
	std::cout << "Saving window bounds." << std::endl;
	std::cout << std::endl;

	File file = getSavedStateDirectory().getChildFile("windowState.xml");

	XmlElement* xml = new XmlElement("MAINWINDOW");

	xml->setAttribute("version", JUCEApplication::getInstance()->getApplicationVersion());
	xml->setAttribute("shouldReloadOnStartup", shouldReloadOnStartup);

	XmlElement* bounds = new XmlElement("BOUNDS");
	bounds->setAttribute("x",getScreenX());
	bounds->setAttribute("y",getScreenY());
	bounds->setAttribute("w",getContentComponent()->getWidth());
	bounds->setAttribute("h",getContentComponent()->getHeight());
	bounds->setAttribute("fullscreen", isFullScreen());

	xml->addChildElement(bounds);

	XmlElement* recentDirectories = new XmlElement("RECENTDIRECTORYNAMES");

	UIComponent* ui = (UIComponent*) getContentComponent();

	StringArray dirs = ui->getRecentlyUsedFilenames();

	for (int i = 0; i < dirs.size(); i++)
	{
		XmlElement* directory = new XmlElement("DIRECTORY");
		directory->setAttribute("name", dirs[i]);
		recentDirectories->addChildElement(directory);
	}

	xml->addChildElement(recentDirectories);

	String error;

	if (! xml->writeToFile(file, String::empty))
		error = "Couldn't write to file";

	delete xml;
}

void MainWindow::loadWindowBounds()
{

	std::cout << std::endl;
	std::cout << "Loading window bounds." << std::endl;
	std::cout << std::endl;

	File file = getSavedStateDirectory().getChildFile("windowState.xml");

	XmlDocument doc(file);
	XmlElement* xml = doc.getDocumentElement();

	if (xml == 0 || ! xml->hasTagName("MAINWINDOW"))
	{

		std::cout << "File not found." << std::endl;
		delete xml;
		centreWithSize(800, 600);

	}
	else
	{

		String description;

		shouldReloadOnStartup = xml->getBoolAttribute("shouldReloadOnStartup", false);

		forEachXmlChildElement(*xml, e)
		{

			if (e->hasTagName("BOUNDS"))
			{

				int x = e->getIntAttribute("x");
				int y = e->getIntAttribute("y");
				int w = e->getIntAttribute("w");
				int h = e->getIntAttribute("h");

				// bool fs = e->getBoolAttribute("fullscreen");

				// without the correction, you get drift over time
#ifdef WIN32
				setTopLeftPosition(x,y); //Windows doesn't need correction
#else
				setTopLeftPosition(x,y-27);
#endif
				getContentComponent()->setBounds(0,0,w-10,h-33);
				//setFullScreen(fs);
			}
			else if (e->hasTagName("RECENTDIRECTORYNAMES"))
			{

				StringArray filenames;

				forEachXmlChildElement(*e, directory)
				{

					if (directory->hasTagName("DIRECTORY"))
					{
						filenames.add(directory->getStringAttribute("name"));
					}
				}

				UIComponent* ui = (UIComponent*) getContentComponent();
				ui->setRecentlyUsedFilenames(filenames);

			}

		}

		delete xml;
	}
	// return "Everything went ok.";
}

<<<<<<< HEAD
UIBackgroundLoader::UIBackgroundLoader(UIComponent* ui, File loadFile) : Thread("Load UI"), ui(ui), loadFile(loadFile)
=======
UILoadInBackground::UILoadInBackground(UIComponent* ui, File loadFile) : Thread("Load UI"), ui(ui), loadFile(loadFile)
>>>>>>> 583d0798e6b9f3c4d60798f2dd2100c4ce3c4ba3
{

}

<<<<<<< HEAD
UIBackgroundLoader::~UIBackgroundLoader()
=======
UILoadInBackground::~UILoadInBackground()
>>>>>>> 583d0798e6b9f3c4d60798f2dd2100c4ce3c4ba3
{

}

<<<<<<< HEAD
void UIBackgroundLoader::run()
{
	CoreServices::sendStatusMessage("Loading last used signal chain...");
	wait(10); //HACK FOR NOW, should wait exactly until MessageManagerLock is available...
	const MessageManagerLock mmLock;
	ui->getEditorViewport()->loadState(loadFile);
	CoreServices::sendStatusMessage("Finished loading signal chain.");
=======
void UILoadInBackground::run()
{
	std::cout << "Loading from seperate thread!" << std::endl;
	wait(2000); //HACK FOR NOW, should wait until MessageManagerLock is available...
	//Now the main UI has loaded and now we need to add some feedback indicating the UI is loading a signal chain
	const MessageManagerLock mmLock;
	ui->getEditorViewport()->loadState(loadFile);
>>>>>>> 583d0798e6b9f3c4d60798f2dd2100c4ce3c4ba3
}
