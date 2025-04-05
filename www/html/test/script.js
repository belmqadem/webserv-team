// Function to fetch and display files for deletion
async function showDeleteOptions() {
  const response = await fetch("/list_files.php");
  if (response.ok) {
    const files = await response.json();
    const select = document.getElementById("delete");
    select.innerHTML = "";
    files.forEach((file) => {
      const option = document.createElement("option");
      option.value = file;
      option.textContent = file;
      select.appendChild(option);
    });
  } else {
    console.error("Failed to fetch files for deletion.");
  }
}

// Function to send DELETE request
async function deleteFile() {
  const selectedFile = document.getElementById("delete").value;
  if (!selectedFile) {
    alert("Please select a file to delete.");
    return;
  }
  const response = await fetch("/delete/" + selectedFile, {
    method: "DELETE",
  });
  if (response.ok) {
    alert(`File ${selectedFile} deleted successfully.`);
    showDeleteOptions();
  } else {
    alert(`Failed to delete ${selectedFile}.`);
  }
}

// Load files when the page is ready
window.onload = () => {
  showDeleteOptions();
  document
    .getElementById("delete-button")
    .addEventListener("click", deleteFile);
};
