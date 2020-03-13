/*
	 ------------------------------------------------------------------

	 This file is part of the Open Ephys GUI
	 Copyright (C) 2019 Open Ephys

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

#include "PluginInstaller.h"
#include <stdio.h>
//-----------------------------------------------------------------------

static inline File getPluginsLocationDirectory() {
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

PluginInstaller::PluginInstaller(MainWindow* mainWindow)
: DocumentWindow(WINDOW_TITLE,
		Colour(Colours::black),
		DocumentWindow::closeButton)
{

	parent = (DocumentWindow*)mainWindow;

	setResizable(
        true,  // isResizable
		false); // useBottomCornerRisizer -- doesn't work very well

    //TODO: Add command manager for hot-key functionality later...

    /*
	commandManager.registerAllCommandsForTarget(ui);
	commandManager.registerAllCommandsForTarget(JUCEApplication::getInstance());
	ui->setApplicationCommandManagerToWatch(&commandManager);
	addKeyListener(commandManager.getKeyMappings());
    */

	setUsingNativeTitleBar(true);
	setContentOwned(new PluginInstallerComponent(), true);
	setVisible(true);
	
	int x = parent->getX();
	int y = parent->getY();
	int w = parent->getWidth();
	int h = parent->getHeight();

	setBounds(x+0.1*w, y+0.25*h, 0.8*w, 0.5*h);

	// Constraining the window's size doesn't seem to work:
	setResizeLimits(640, 480, 8192, 5120);
}

PluginInstaller::~PluginInstaller()
{
	masterReference.clear();
}

void PluginInstaller::closeButtonPressed()
{
	setVisible(false);
	delete this;
}


/* ================================== Plugin Installer Component ================================== */

PluginInstallerComponent::PluginInstallerComponent()
{
	font = Font(Font::getDefaultSansSerifFontName(), 16, Font::plain);
	setSize(getWidth() - 10, getHeight() - 10);

	addAndMakeVisible(pluginListAndInfo);

	addAndMakeVisible(sortingLabel);
	sortingLabel.setColour(Label::textColourId, Colours::white);
	sortingLabel.setFont(font);
	sortingLabel.setText("Sort By:", dontSendNotification);

	addAndMakeVisible(sortByMenu);
	sortByMenu.setJustificationType(Justification::centred);
	sortByMenu.addItem("Ascending", 1);
	sortByMenu.addItem("Descending", 2);
	sortByMenu.setTextWhenNothingSelected("------");
	sortByMenu.addListener(this);
}

void PluginInstallerComponent::paint(Graphics& g)
{
	g.fillAll (Colours::darkgrey);
}

void PluginInstallerComponent::resized()
{
	sortingLabel.setBounds(20, 10, 70, 30);
	sortByMenu.setBounds(90, 10, 110, 30);
	pluginListAndInfo.setBounds(10, 40, getWidth() - 10, getHeight() - 40);
}

void PluginInstallerComponent::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
	if (comboBoxThatHasChanged->getSelectedId() == 1)
	{
		pluginListAndInfo.pluginArray.sort(true);
		pluginListAndInfo.repaint();
	}
	else if (comboBoxThatHasChanged->getSelectedId() == 2)
	{
		pluginListAndInfo.pluginArray.sort(true);
		int size = pluginListAndInfo.pluginArray.size();
		for (int i = 0; i < size / 2; i++)
		{
			pluginListAndInfo.pluginArray.getReference(i).swapWith
			(pluginListAndInfo.pluginArray.getReference(size - i - 1));
		}

		pluginListAndInfo.repaint();
	}
}


/* ================================== Plugin Table Component ================================== */

PluginListBoxComponent::PluginListBoxComponent()
{
	listFont = Font(Font::getDefaultSansSerifFontName(), 20, Font::plain);
	listFont.setHorizontalScale(1.1);

	loadPluginNames();

	addAndMakeVisible(pluginList);
	pluginList.setModel(this);
	pluginList.setColour(ListBox::backgroundColourId , Colours::grey);
	pluginList.setRowHeight(35);

	addAndMakeVisible(pluginInfoPanel);
}

int PluginListBoxComponent::getNumRows()
{
	return numRows;
}

void PluginListBoxComponent::paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected)
{
	if (rowIsSelected)
	{
		g.fillAll(Colours::azure);
		g.setColour (Colours::grey);
	}
	else
	{
		g.fillAll(Colours::grey);
		g.setColour (Colours::white);
	}

	g.setFont(listFont);

	String text = pluginArray[rowNumber];

	g.drawText (text, 20, 0, width - 10, height, Justification::centredLeft, true);
}

void PluginListBoxComponent::loadPluginNames()
{
	/* Get list of plugins uploaded to bintray */
	String response = URL("https://api.bintray.com/repos/open-ephys-gui-plugins").readEntireTextStream();

	pluginData = JSON::parse(response);

	numRows = pluginData.size();
	
	// std::cout << "jsonReply" << response.bodyAsString << std::endl;

	String pluginName;

	for (int i = 0; i < numRows; i++)
	{
		//Array<String> packages;
		
		pluginName = pluginData[i].getProperty("name", var()).toString();
		pluginArray.add(pluginName);
	}
}

bool PluginListBoxComponent::loadPluginInfo(const String& pluginName)
{
	// Find out all available packages for the plugin
	String url, version_url;
	
	url = "https://api.bintray.com/repos/open-ephys-gui-plugins/";
	url+=pluginName;
	url+="/packages";

	String packageResponse = URL(url).readEntireTextStream();

	var packageReply = JSON::parse(packageResponse);

	Array<String> packages;

	for (int i = 0; i < packageReply.size(); i++)
	{
	 	packages.add(packageReply[i].getProperty("name", var()).toString());
	}


	// Identify the OS on which the GUI is running
	juce::SystemStats::OperatingSystemType os = juce::SystemStats::getOperatingSystemType();
	String os_name;

	if ((os & juce::SystemStats::OperatingSystemType::Windows) != 0)
	 	os_name = "windows";
	else if ((os & juce::SystemStats::OperatingSystemType::MacOSX) != 0)
	 	os_name = "mac";
	else if ((os & juce::SystemStats::OperatingSystemType::Linux) != 0)
	 	os_name = "linux";

		
	// Select platform specific package for the plugin
	String selectedPackage;
	for (int i = 0; i < packages.size(); i++)
	{
	 	if(packages[i].contains(os_name))
	 	{
	 		selectedPackage = packages[i];
	 		break;
	 	}
	}

	if(selectedPackage.isEmpty())
	{
	 	std::cout << "*********** No platform specific package found for " << pluginName << std::endl;
		pluginInfoPanel.makeInfoVisible(false);
		return false;
	}

	//Get latest version
	version_url = "https://api.bintray.com/packages/open-ephys-gui-plugins/";
	version_url+=pluginName;
	version_url+="/";
	version_url+=selectedPackage;

	String version_response = URL(version_url).readEntireTextStream();;

	var version_reply = JSON::parse(version_response);

	String owner= version_reply.getProperty("owner", "NULL");
	String latest_version = version_reply.getProperty("latest_version", "NULL");
	String updated = version_reply.getProperty("updated", "NULL");
	String description = version_reply.getProperty("desc", "NULL");

	auto allVersions = version_reply.getProperty("versions", "NULL").getArray();

	selectedPluginInfo.versions.clear();

	for (String version : *allVersions)
		selectedPluginInfo.versions.add(version);

	selectedPluginInfo.docURL = version_reply.getProperty("vcs_url", "NULL").toString();
	selectedPluginInfo.selectedVersion = String();

	selectedPluginInfo.pluginName = pluginName;
	selectedPluginInfo.packageName = selectedPackage;
	selectedPluginInfo.owner = owner;
	selectedPluginInfo.latestVersion = latest_version;
	selectedPluginInfo.lastUpdated = updated;
	selectedPluginInfo.description = description;
	selectedPluginInfo.dependencies = "Dependencies: ";
	
	pluginInfoPanel.setPluginInfo(selectedPluginInfo);
	pluginInfoPanel.makeInfoVisible(true);

	return true;
}

void PluginListBoxComponent::listBoxItemClicked (int row, const MouseEvent &)
{
	String pName = pluginArray[row];
	if ( !pName.equalsIgnoreCase(pluginInfoPanel.getSelectedPlugin()) )
	{
		pluginInfoPanel.makeInfoVisible(false);
		pluginInfoPanel.updateStatusMessage("Loading Plugin Info...", true);
		
		if(loadPluginInfo(pName))
			pluginInfoPanel.updateStatusMessage("", false);
		else
			pluginInfoPanel.updateStatusMessage("No platform specific package found for " + pName, true);
	}
}

void PluginListBoxComponent::resized()
{
	// position our table with a gap around its edge
    pluginList.setBounds(10, 10, getWidth() - (0.5 * getWidth()) - 20, getHeight() - 30);
	pluginInfoPanel.setBounds(getWidth() - (0.5 * getWidth()), 10, getWidth() - (0.5 * getWidth()) - 20, getHeight() - 30);
}

/* ================================== Plugin Information Component ================================== */

PluginInfoComponent::PluginInfoComponent()
{
	infoFont = Font(Font::getDefaultSansSerifFontName(), 19, Font::plain);
	
	addChildComponent(pluginNameLabel);
	pluginNameLabel.setFont(infoFont);
	pluginNameLabel.setColour(Label::textColourId, Colours::white);

	addChildComponent(ownerLabel);
	ownerLabel.setFont(infoFont);
	ownerLabel.setColour(Label::textColourId, Colours::white);

	addChildComponent(versionLabel);
	versionLabel.setFont(infoFont);
	versionLabel.setColour(Label::textColourId, Colours::white);
	versionLabel.setText("Version: ", dontSendNotification);

	addChildComponent(versionMenu);
	versionMenu.setJustificationType(Justification::centred);
	versionMenu.setTextWhenNothingSelected("------");
	versionMenu.addListener(this);

	addChildComponent(lastUpdatedLabel);
	lastUpdatedLabel.setFont(infoFont);
	lastUpdatedLabel.setColour(Label::textColourId, Colours::white);

	addChildComponent(descriptionLabel);
	descriptionLabel.setFont(infoFont);
	descriptionLabel.setColour(Label::textColourId, Colours::white);
	descriptionLabel.setText("Description: ", dontSendNotification);

	addChildComponent(descriptionText);
	descriptionText.setFont(infoFont);
	descriptionText.setColour(Label::textColourId, Colours::white);
	descriptionText.setJustificationType(Justification::horizontallyJustified | Justification::top);

	addChildComponent(dependencyLabel);
	dependencyLabel.setFont(infoFont);
	dependencyLabel.setColour(Label::textColourId, Colours::white);

	addChildComponent(downloadButton);
	downloadButton.setButtonText("Download & Install");
	downloadButton.setColour(TextButton::buttonColourId, Colours::lightgrey);
	downloadButton.addListener(this);
	downloadButton.setEnabled(false);

	addChildComponent(documentationButton);
	documentationButton.setButtonText("Open Documentation");
	documentationButton.setColour(TextButton::buttonColourId, Colours::lightgrey);
	documentationButton.addListener(this);

	addAndMakeVisible(statusLabel);
	statusLabel.setFont(infoFont);
	statusLabel.setColour(Label::textColourId, Colours::white);
	statusLabel.setText("Please select a plugin from the list on the left...", dontSendNotification);
}

void PluginInfoComponent::paint(Graphics& g)
{
	g.fillAll (Colours::grey);
	g.setFont(infoFont);
}

void PluginInfoComponent::resized()
{
	pluginNameLabel.setBounds(10, 30, getWidth() - 10, 30);
	ownerLabel.setBounds(10, 60, getWidth() - 10, 30);
	versionLabel.setBounds(10, 90, 80, 30);
	versionMenu.setBounds(90, 90, 100, 30);
	lastUpdatedLabel.setBounds(10, 120, getWidth() - 10, 30);
	descriptionLabel.setBounds(10, 150, 110, 30);
	descriptionText.setBounds(120, 155, getWidth() - 130, 50);
	dependencyLabel.setBounds(10, 160 + descriptionText.getHeight(), getWidth() - 10, 30);
	downloadButton.setBounds(getWidth() - (getWidth() * 0.4) - 20, getHeight() - 60, getWidth() * 0.4, 30);
	documentationButton.setBounds(20, getHeight() - 60, getWidth() * 0.4, 30);
	statusLabel.setBounds(10, getHeight() / 2, getWidth() - 10, 30);
}

void PluginInfoComponent::buttonClicked(Button* button)
{
	if (button == &downloadButton)
	{
		std::cout << "Downloading Plugin: " << pInfo.pluginName << "...  ";
		
		bool dlSucess = downloadPlugin(pInfo.pluginName, pInfo.packageName, pInfo.selectedVersion);

		if(dlSucess)
		{	
			juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, 
												   "Plugin Installer " + pInfo.pluginName, 
												   pInfo.pluginName + " Installed Successfully");

			std::cout << "Download Successfull!!" << std::endl;
		}
		else
		{
			juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, 
												   "Plugin Installer " + pInfo.pluginName, 
												   "Error Installing " + pInfo.pluginName);

			std::cout << "Download Failed!!" << std::endl;;
		}
		
	}
	else if (button == &documentationButton)
	{
		URL url = URL(pInfo.docURL);
		url.launchInDefaultBrowser();
	}
}

void PluginInfoComponent::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
	if (comboBoxThatHasChanged == &versionMenu)
	{
		pInfo.selectedVersion = comboBoxThatHasChanged->getText();
		downloadButton.setEnabled(true);
	}
}

void PluginInfoComponent::setPluginInfo(const SelectedPluginInfo& p)
{
	pInfo = p;
	pluginNameLabel.setText("Name: " + pInfo.pluginName, dontSendNotification);
	ownerLabel.setText("Author: " + pInfo.owner, dontSendNotification);
	lastUpdatedLabel.setText("Last Updated: " + pInfo.lastUpdated, dontSendNotification);
	descriptionText.setText(pInfo.description, dontSendNotification);
	dependencyLabel.setText(pInfo.dependencies, dontSendNotification);

	versionMenu.clear(dontSendNotification);

	for (int i = 0; i < pInfo.versions.size(); i++)
		versionMenu.addItem(pInfo.versions[i], i + 1);

	downloadButton.setEnabled(false);
}

void PluginInfoComponent::updateStatusMessage(const String& str, bool isVisible)
{
	statusLabel.setText(str, dontSendNotification);
	statusLabel.setVisible(isVisible);
}

void PluginInfoComponent::makeInfoVisible(bool isEnabled)
{
	pluginNameLabel.setVisible(isEnabled);
	ownerLabel.setVisible(isEnabled);
	versionLabel.setVisible(isEnabled);
	versionMenu.setVisible(isEnabled);
	lastUpdatedLabel.setVisible(isEnabled);
	descriptionLabel.setVisible(isEnabled);
	descriptionText.setVisible(isEnabled);
	dependencyLabel.setVisible(isEnabled);
	downloadButton.setVisible(isEnabled);
	documentationButton.setVisible(isEnabled);
}

bool PluginInfoComponent::downloadPlugin(const String& plugin, const String& package, const String& version) 
{

	String files_url = "https://api.bintray.com/packages/open-ephys-gui-plugins/";
	files_url += plugin;
	files_url += "/";
	files_url += package;
	files_url += "/versions/";
	files_url += version;
	files_url += "/files";

	String files_response = URL(files_url).readEntireTextStream();;

	var files_reply = JSON::parse(files_response);

	String filename;

	if (files_reply.size())
	{
		for (int i = 0; i < files_reply.size(); i++)
		{
			filename = files_reply[i].getProperty("name", "NULL").toString();
		}

		//Unzip plugin and install in plugins directory
		//curl -L https://dl.bintray.com/$bintrayUser/$repo/$filename
		String fileDownloadURL = "https://dl.bintray.com/open-ephys-gui-plugins/";
		fileDownloadURL += plugin;
		fileDownloadURL += "/";
		fileDownloadURL += filename;

		URL fileUrl(fileDownloadURL);

		ScopedPointer<InputStream> fileStream = fileUrl.createInputStream(false);

		//Get path to plugins directory
		File pluginsPath = getPluginsLocationDirectory();

		//Construct path for downloaded zip file
		String pluginFilePath = pluginsPath.getFullPathName();
		pluginFilePath += File::separatorString;
		pluginFilePath += filename;

		//Create local file
		File pluginFile(pluginFilePath);
		pluginFile.deleteFile();

		juce::FileOutputStream out(pluginFile);
		out.writeFromInputStream(*fileStream, -1);
		out.flush();

		juce::ZipFile pluginZip(pluginFile);
		Result rs = pluginZip.uncompressTo(pluginsPath);

		pluginFile.deleteFile();		

		if (rs.failed())
			return false;

		return true;

	}
	//TODO: Prompt user to restart to see plugin in ProcessorList

	return false;
}