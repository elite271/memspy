#include "ScannerWindow.h"
#include <charconv>

ScannerWindow::ScannerWindow()
{
}

ScannerWindow::~ScannerWindow()
{
}

bool ScannerWindow::Init()
{
	return true;
}

void ScannerWindow::OnWindowResize(float width, float height)
{
	ImGui::SetNextWindowSize(ImVec2(width, height));
}

void ScannerWindow::ResetScanState()
{
	scanResults.clear();
	hasScannedOnce = false;
	currentScanType = FirstScan;
	inputText.clear();
}

void ScannerWindow::OnScanButtonPressed(ProcessHandle& processHandle, MemoryRegions& regions)
{
	// Validation - input required for first scan or exact value scan
	if ((!hasScannedOnce || currentScanType == ExactValue) && inputText.empty())
	{
		printf("Input is empty!\n");
		return;
	}

	isScanning = true;
	std::vector<ScanResult> newResults;

	if (!hasScannedOnce)
	{
		// FIRST SCAN - scan all memory
		scanResults.clear();

		// Handle string scanning
		if (currentScanSize == String)
		{
			scanResults = ScanUtils::ScanForString(processHandle, regions, inputText, true);
			std::cout << "String scan complete. Found " << scanResults.size() << " matches." << std::endl;
			hasScannedOnce = true;
			currentScanType = ExactValue; // Default for next scan
			isScanning = false;
			return;
		}

		// Handle numeric scanning
		try
		{
			switch (currentScanSize)
			{
			case Byte:
			{
				int value = std::stoi(inputText);
				if (value < 0 || value > 255)
				{
					printf("Byte value must be 0-255\n");
					isScanning = false;
					return;
				}
				scanResults = ScanUtils::ScanForValue<uint8_t>(processHandle, regions, static_cast<uint8_t>(value));
				break;
			}
			case TwoBytes:
			{
				int value = std::stoi(inputText);
				scanResults = ScanUtils::ScanForValue<int16_t>(processHandle, regions, static_cast<int16_t>(value));
				break;
			}
			case FourBytes:
			{
				int32_t value = std::stoi(inputText);
				scanResults = ScanUtils::ScanForValue<int32_t>(processHandle, regions, value);
				break;
			}
			case EightBytes:
			{
				int64_t value = std::stoll(inputText);
				scanResults = ScanUtils::ScanForValue<int64_t>(processHandle, regions, value);
				break;
			}
			case Float:
			{
				float value = std::stof(inputText);
				scanResults = ScanUtils::ScanForValue<float>(processHandle, regions, value);
				break;
			}
			case Double:
			{
				double value = std::stod(inputText);
				scanResults = ScanUtils::ScanForValue<double>(processHandle, regions, value);
				break;
			}
			}

			std::cout << "Scan complete. Found " << scanResults.size() << " matches." << std::endl;
			hasScannedOnce = true;
			currentScanType = ExactValue; // Default for next scan
		}
		catch (const std::exception& e)
		{
			printf("Error parsing input: %s\n", e.what());
		}
	}
	else
	{
		// NEXT SCAN - filter previous results
		try
		{
			switch (currentScanSize)
			{
			case Byte:
			{
				if (currentScanType == ExactValue)
				{
					int value = std::stoi(inputText);
					if (value < 0 || value > 255)
					{
						printf("Byte value must be 0-255\n");
						isScanning = false;
						return;
					}
					newResults = ScanUtils::NextScan<uint8_t>(processHandle, scanResults, currentScanType, static_cast<uint8_t>(value));
				}
				else
				{
					newResults = ScanUtils::NextScan<uint8_t>(processHandle, scanResults, currentScanType, std::nullopt);
				}
				break;
			}
			case TwoBytes:
			{
				if (currentScanType == ExactValue)
				{
					int value = std::stoi(inputText);
					newResults = ScanUtils::NextScan<int16_t>(processHandle, scanResults, currentScanType, static_cast<int16_t>(value));
				}
				else
				{
					newResults = ScanUtils::NextScan<int16_t>(processHandle, scanResults, currentScanType, std::nullopt);
				}
				break;
			}
			case FourBytes:
			{
				if (currentScanType == ExactValue)
				{
					int32_t value = std::stoi(inputText);
					newResults = ScanUtils::NextScan<int32_t>(processHandle, scanResults, currentScanType, value);
				}
				else
				{
					newResults = ScanUtils::NextScan<int32_t>(processHandle, scanResults, currentScanType, std::nullopt);
				}
				break;
			}
			case EightBytes:
			{
				if (currentScanType == ExactValue)
				{
					int64_t value = std::stoll(inputText);
					newResults = ScanUtils::NextScan<int64_t>(processHandle, scanResults, currentScanType, value);
				}
				else
				{
					newResults = ScanUtils::NextScan<int64_t>(processHandle, scanResults, currentScanType, std::nullopt);
				}
				break;
			}
			case Float:
			{
				if (currentScanType == ExactValue)
				{
					float value = std::stof(inputText);
					newResults = ScanUtils::NextScan<float>(processHandle, scanResults, currentScanType, value);
				}
				else
				{
					newResults = ScanUtils::NextScan<float>(processHandle, scanResults, currentScanType, std::nullopt);
				}
				break;
			}
			case Double:
			{
				if (currentScanType == ExactValue)
				{
					double value = std::stod(inputText);
					newResults = ScanUtils::NextScan<double>(processHandle, scanResults, currentScanType, value);
				}
				else
				{
					newResults = ScanUtils::NextScan<double>(processHandle, scanResults, currentScanType, std::nullopt);
				}
				break;
			}
			}

			scanResults = newResults;
			std::cout << "Next scan complete. Found " << scanResults.size() << " matches." << std::endl;
		}
		catch (const std::exception& e)
		{
			printf("Error parsing input: %s\n", e.what());
		}
	}

	isScanning = false;
}

void ScannerWindow::Render(ProcessHandle& processHandle, MemoryRegions* regions)
{
	static const char* items[] = { "Byte", "2 Bytes", "4 Bytes", "8 Bytes", "Float", "Double", "String", "WString"};
	static int current_item_idx = 0;

	static const char* scanTypeItems[] = {
		"Exact Value", "Changed Value", "Unchanged Value",
		"Increased Value", "Decreased Value"
	};
	static int scanTypeIdx = 0;

	// Calculate available width for responsive layout
	float availWidth = ImGui::GetContentRegionAvail().x;
	float comboWidth = 120.0f; // Fixed width for combo
	float scanTypeWidth = 150.0f; // Width for scan type combo
	float spacing = ImGui::GetStyle().ItemSpacing.x;

	// Show scan type combo after first scan
	if (hasScannedOnce)
	{
		ImGui::SetNextItemWidth(scanTypeWidth);
		if (ImGui::Combo("##ScanType", &scanTypeIdx, scanTypeItems, IM_ARRAYSIZE(scanTypeItems)))
		{
			currentScanType = static_cast<ScanType>(scanTypeIdx + 1); // +1 to skip FirstScan
		}
		ImGui::SameLine();
		availWidth -= scanTypeWidth + spacing;
	}

	float inputWidth = availWidth - comboWidth - spacing;

	// Only show input for FirstScan or ExactValue
	if (!hasScannedOnce || currentScanType == ExactValue)
	{
		ImGui::SetNextItemWidth(inputWidth);
		ImGui::InputText("##ScanFor", &inputText);
		ImGui::SameLine();
	}

	ImGui::SetNextItemWidth(comboWidth);
	ImGui::BeginDisabled(hasScannedOnce);
	if (ImGui::Combo("##DataSize", &current_item_idx, items, IM_ARRAYSIZE(items)))
	{
		currentScanSize = static_cast<ScanSize>(current_item_idx);
		std::cout << items[current_item_idx] << std::endl;
	}
	ImGui::EndDisabled();

	const char* buttonText = hasScannedOnce ? "Next Scan" : "First Scan";
	if (ImGui::Button(buttonText) && regions && regions->isValid())
	{
		OnScanButtonPressed(processHandle, *regions);
	}

	if (hasScannedOnce)
	{
		ImGui::SameLine();
		if (ImGui::Button("New Scan"))
		{
			ResetScanState();
			scanTypeIdx = 0; // Reset scan type dropdown
		}
	}

	ImGui::SameLine();
	ImGui::Text("Results: %zu", scanResults.size());

	ImGui::Separator();

	// Display scan results in a table
	if (!scanResults.empty())
	{
		if (ImGui::BeginTable("ResultsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable))
		{
			ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 150.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableHeadersRow();

			// Limit display to first 10000 results for performance
			const size_t displayLimit = std::min<size_t>(10000, scanResults.size());

			for (size_t i = 0; i < displayLimit; i++)
			{
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::Text("0x%016llX", scanResults[i].address);

				ImGui::TableSetColumnIndex(1);

				// Read and display the actual value at this address
				SIZE_T bytesRead = 0;
				bool readSuccess = false;

				switch (currentScanSize)
				{
				case Byte:
				{
					uint8_t value = 0;
					readSuccess = ReadProcessMemory(processHandle.GetHandle(),
						reinterpret_cast<LPCVOID>(scanResults[i].address),
						&value, sizeof(value), &bytesRead);
					if (readSuccess)
					{
						ImGui::Text("%u", value);
						if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
						{
							selectedResultIndex = i;
							snprintf(editBuffer, sizeof(editBuffer), "%u", value);
							showEditPopup = true;
						}
					}
					break;
				}
				case TwoBytes:
				{
					int16_t value = 0;
					readSuccess = ReadProcessMemory(processHandle.GetHandle(),
						reinterpret_cast<LPCVOID>(scanResults[i].address),
						&value, sizeof(value), &bytesRead);
					if (readSuccess)
					{
						ImGui::Text("%d", value);
						if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
						{
							selectedResultIndex = i;
							snprintf(editBuffer, sizeof(editBuffer), "%d", value);
							showEditPopup = true;
						}
					}
					break;
				}
				case FourBytes:
				{
					int32_t value = 0;
					readSuccess = ReadProcessMemory(processHandle.GetHandle(),
						reinterpret_cast<LPCVOID>(scanResults[i].address),
						&value, sizeof(value), &bytesRead);
					if (readSuccess)
					{
						ImGui::Text("%d", value);
						if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
						{
							selectedResultIndex = i;
							snprintf(editBuffer, sizeof(editBuffer), "%d", value);
							showEditPopup = true;
						}
					}
					break;
				}
				case EightBytes:
				{
					int64_t value = 0;
					readSuccess = ReadProcessMemory(processHandle.GetHandle(),
						reinterpret_cast<LPCVOID>(scanResults[i].address),
						&value, sizeof(value), &bytesRead);
					if (readSuccess)
					{
						ImGui::Text("%lld", value);
						if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
						{
							selectedResultIndex = i;
							snprintf(editBuffer, sizeof(editBuffer), "%lld", value);
							showEditPopup = true;
						}
					}
					break;
				}
				case Float:
				{
					float value = 0.0f;
					readSuccess = ReadProcessMemory(processHandle.GetHandle(),
						reinterpret_cast<LPCVOID>(scanResults[i].address),
						&value, sizeof(value), &bytesRead);
					if (readSuccess)
					{
						ImGui::Text("%.2f", value);
						if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
						{
							selectedResultIndex = i;
							snprintf(editBuffer, sizeof(editBuffer), "%.2f", value);
							showEditPopup = true;
						}
					}
					break;
				}
				case Double:
				{
					double value = 0.0;
					readSuccess = ReadProcessMemory(processHandle.GetHandle(),
						reinterpret_cast<LPCVOID>(scanResults[i].address),
						&value, sizeof(value), &bytesRead);
					if (readSuccess)
					{
						ImGui::Text("%.2f", value);
						if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
						{
							selectedResultIndex = i;
							snprintf(editBuffer, sizeof(editBuffer), "%.2f", value);
							showEditPopup = true;
						}
					}
					break;
				}
				case String:
				{
					char buffer[64] = {};
					readSuccess = ReadProcessMemory(processHandle.GetHandle(),
						reinterpret_cast<LPCVOID>(scanResults[i].address),
						buffer, sizeof(buffer) - 1, &bytesRead);
					if (readSuccess)
					{
						buffer[bytesRead] = '\0';
						ImGui::Text("%s", buffer);
					}
					break;
				}
				}

				if (!readSuccess)
				{
					ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "???");
				}
			}

			if (scanResults.size() > displayLimit)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "... %zu more results not shown", scanResults.size() - displayLimit);
			}

			ImGui::EndTable();
		}
	}
	else if (isScanning)
	{
		ImGui::Text("Scanning...");
	}
	else
	{
		ImGui::Text("No results. Enter a value and click Scan.");
	}

	// Edit value popup
	if (showEditPopup)
	{
		ImGui::OpenPopup("Edit Value");
		showEditPopup = false;
	}

	if (ImGui::BeginPopupModal("Edit Value", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (selectedResultIndex >= 0 && selectedResultIndex < scanResults.size())
		{
			ImGui::Text("Address: 0x%016llX", scanResults[selectedResultIndex].address);
			ImGui::Separator();

			ImGui::InputText("New Value", editBuffer, sizeof(editBuffer));

			ImGui::Separator();

			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				// Write the new value
				try
				{
					SIZE_T bytesWritten = 0;
					bool writeSuccess = false;

					switch (currentScanSize)
					{
					case Byte:
					{
						uint8_t newValue = static_cast<uint8_t>(std::stoi(editBuffer));
						writeSuccess = WriteProcessMemory(processHandle.GetHandle(), reinterpret_cast<LPVOID>(scanResults[selectedResultIndex].address), &newValue, sizeof(newValue), &bytesWritten);
						break;
					}
					case TwoBytes:
					{
						int16_t newValue = static_cast<int16_t>(std::stoi(editBuffer));
						writeSuccess = WriteProcessMemory(processHandle.GetHandle(), reinterpret_cast<LPVOID>(scanResults[selectedResultIndex].address), &newValue, sizeof(newValue), &bytesWritten);
						break;
					}
					case FourBytes:
					{
						int32_t newValue = std::stoi(editBuffer);
						writeSuccess = WriteProcessMemory(processHandle.GetHandle(), reinterpret_cast<LPVOID>(scanResults[selectedResultIndex].address), &newValue, sizeof(newValue), &bytesWritten);
						break;
					}
					case EightBytes:
					{
						int64_t newValue = std::stoll(editBuffer);
						writeSuccess = WriteProcessMemory(processHandle.GetHandle(), reinterpret_cast<LPVOID>(scanResults[selectedResultIndex].address), &newValue, sizeof(newValue), &bytesWritten);
						break;
					}
					case Float:
					{
						float newValue = std::stof(editBuffer);
						writeSuccess = WriteProcessMemory(processHandle.GetHandle(), reinterpret_cast<LPVOID>(scanResults[selectedResultIndex].address), &newValue, sizeof(newValue), &bytesWritten);
						break;
					}
					case Double:
					{
						double newValue = std::stod(editBuffer);
						writeSuccess = WriteProcessMemory(processHandle.GetHandle(), reinterpret_cast<LPVOID>(scanResults[selectedResultIndex].address), &newValue, sizeof(newValue), &bytesWritten);
						break;
					}
					}

					if (writeSuccess)
					{
						printf("Successfully wrote value to 0x%llX\n", scanResults[selectedResultIndex].address);
					}
					else
					{
						printf("Failed to write to 0x%llX\n", scanResults[selectedResultIndex].address);
					}
				}
				catch (const std::exception& e)
				{
					printf("Error parsing value: %s\n", e.what());
				}

				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::EndPopup();
	}

}
