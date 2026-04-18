import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox
from animal_shelter import AnimalShelter


class AnimalShelterApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Animal Shelter CRUD Viewer")
        self.root.geometry("900x600")

        self.collapsed_view = False
        self.last_displayed_records = []

        # create an AnimalShelter instance
        try:
            self.shelter = AnimalShelter()
        except Exception as e:
            messagebox.showerror("Connection Error", f"Failed to connect to MongoDB:\n{e}")
            self.root.destroy()
            return

        self.build_gui()

    def build_gui(self):
        # container for input box and buttons
        control_frame = ttk.Frame(self.root, padding=10)
        control_frame.pack(fill="x")

        # search bar
        ttk.Label(control_frame, text="Search by Name:").grid(row=0, column=0, padx=5, pady=5, sticky="w")
        self.name_entry = ttk.Entry(control_frame, width=30)
        self.name_entry.grid(row=0, column=1, padx=5, pady=5, sticky="w")

        # buttons
        ttk.Button(control_frame, text="Preview Records", command=self.preview_records).grid(row=0, column=2, padx=5,
                                                                                             pady=5)
        ttk.Button(control_frame, text="Load All Records", command=self.load_all_records).grid(row=0, column=3, padx=5,
                                                                                               pady=5)
        ttk.Button(control_frame, text="Search Name", command=self.search_by_name).grid(row=0, column=4, padx=5, pady=5)
        ttk.Button(control_frame, text="Clear Output", command=self.clear_output).grid(row=0, column=5, padx=5, pady=5)
        ttk.Button(control_frame, text="Toggle Collapse", command=self.toggle_collapse_view).grid(row=0, column=6, padx=5, pady=5)
        # create fields
        ttk.Label(control_frame, text="Animal ID:").grid(row=1, column=0, padx=5, pady=5, sticky="w")
        self.animal_id_entry = ttk.Entry(control_frame, width=20)
        self.animal_id_entry.grid(row=1, column=1, padx=5, pady=5, sticky="w")

        ttk.Label(control_frame, text="Animal Type:").grid(row=1, column=2, padx=5, pady=5, sticky="w")
        self.animal_type_entry = ttk.Entry(control_frame, width=20)
        self.animal_type_entry.grid(row=1, column=3, padx=5, pady=5, sticky="w")

        ttk.Label(control_frame, text="New Name:").grid(row=2, column=0, padx=5, pady=5, sticky="w")
        self.new_name_entry = ttk.Entry(control_frame, width=20)
        self.new_name_entry.grid(row=2, column=1, padx=5, pady=5, sticky="w")

        ttk.Button(control_frame, text="Create Record", command=self.create_record).grid(row=2, column=2, padx=5, pady=5)

        # delete and update fields
        ttk.Label(control_frame, text="Update Name To:").grid(row=3, column=0, padx=5, pady=5, sticky="w")
        self.update_name_entry = ttk.Entry(control_frame, width=20)
        self.update_name_entry.grid(row=3, column=1, padx=5, pady=5, sticky="w")

        ttk.Button(control_frame, text="Update Record", command=self.update_record).grid(row=3, column=2, padx=5,
                                                                                         pady=5)
        ttk.Button(control_frame, text="Delete Record", command=self.delete_record).grid(row=3, column=3, padx=5,
                                                                                         pady=5)


        # output area
        self.output_box = scrolledtext.ScrolledText(self.root, wrap=tk.WORD, width=100, height=30)
        self.output_box.pack(fill="both", expand=True, padx=10, pady=10)
    #option to load a limited number of records
    def preview_records(self):
        try:
            records = self.shelter.read({}, limit=100)
            self.display_records(records)
        except Exception as e:
            messagebox.showerror("Read Error", f"Failed to read records:\n{e}")
    #option to read all records
    def load_all_records(self):
        try:
            records = self.shelter.read({})
            self.display_records(records)
        except Exception as e:
            messagebox.showerror("Read Error", f"Failed to read records:\n{e}")

    def search_by_name(self):
        name = self.name_entry.get().strip()

        if not name:
            messagebox.showwarning("Input Error", "Please enter a name to search.")
            return

        try:
            records = self.shelter.read({"name": name})
            self.display_records(records)
        except Exception as e:
            messagebox.showerror("Search Error", f"Failed to search records:\n{e}")

    def create_record(self):
        animal_id = self.animal_id_entry.get().strip()
        animal_type = self.animal_type_entry.get().strip()
        name = self.new_name_entry.get().strip()

        if not animal_id or not animal_type or not name:
            messagebox.showwarning("Input Error", "Animal ID, Animal Type, and Name are required.")
            return

        new_record = {
            "animal_id": animal_id,
            "animal_type": animal_type,
            "name": name
        }

        try:
            success = self.shelter.create(new_record)
            if success:
                messagebox.showinfo("Success", "Record created successfully.")
                self.output_box.delete("1.0", tk.END)
                self.output_box.insert(tk.END, f"Created record:\n{new_record}\n")
            else:
                messagebox.showerror("Create Error", "Record was not created. It may already exist.")
        except Exception as e:
            messagebox.showerror("Create Error", f"Failed to create record:\n{e}")

    def update_record(self):
        current_name = self.name_entry.get().strip()
        new_name = self.update_name_entry.get().strip()

        if not current_name or not new_name:
            messagebox.showwarning("Input Error", "Both the search name and updated name are required.")
            return

        try:
            modified_count = self.shelter.update({"name": current_name}, {"name": new_name})

            if modified_count > 0:
                messagebox.showinfo("Success", f"Updated {modified_count} record(s).")
                self.output_box.delete("1.0", tk.END)
                self.output_box.insert(tk.END, f"Updated records with name '{current_name}' to '{new_name}'.\n")
            else:
                messagebox.showinfo("No Changes", "No matching records were updated.")
        except Exception as e:
            messagebox.showerror("Update Error", f"Failed to update record(s):\n{e}")

    def delete_record(self):
        name_to_delete = self.name_entry.get().strip()

        if not name_to_delete:
            messagebox.showwarning("Input Error", "Enter a name in the search box to delete.")
            return

        try:
            # first read matching records so the user can preview what will be deleted
            matching_records = self.shelter.read({"name": name_to_delete})
            match_count = len(matching_records)

            if match_count == 0:
                messagebox.showinfo("No Changes", "No matching records were found to delete.")
                self.output_box.delete("1.0", tk.END)
                self.output_box.insert(tk.END, f"No records found with name '{name_to_delete}'.\n")
                return

            # clear output and show summary
            self.output_box.delete("1.0", tk.END)
            self.output_box.insert(tk.END, f"{match_count} record(s) match the name '{name_to_delete}'.\n")
            self.output_box.insert(tk.END, "-" * 80 + "\n")

            # if the number is small, show details so the user can verify
            if match_count <= 5:
                for i, record in enumerate(matching_records, start=1):
                    self.output_box.insert(tk.END, f"Matching Record {i}:\n")
                    for key, value in record.items():
                        self.output_box.insert(tk.END, f"  {key}: {value}\n")
                    self.output_box.insert(tk.END, "-" * 80 + "\n")
            else:
                self.output_box.insert(
                    tk.END,
                    "Too many records to display safely in full. Please refine your search if needed.\n"
                )

            # ask for confirmation before actually deleting
            confirm = messagebox.askyesno(
                "Confirm Delete",
                f"{match_count} record(s) match '{name_to_delete}'.\n\nDo you want to delete them?"
            )

            if not confirm:
                self.output_box.insert(tk.END, "\nDeletion canceled by user.\n")
                return

            deleted_count = self.shelter.delete({"name": name_to_delete})

            if deleted_count > 0:
                messagebox.showinfo("Success", f"Deleted {deleted_count} record(s).")
                self.output_box.insert(tk.END, f"\nDeleted {deleted_count} record(s) with name '{name_to_delete}'.\n")
            else:
                messagebox.showinfo("No Changes", "No matching records were deleted.")
                self.output_box.insert(tk.END, "\nNo records were deleted.\n")

        except Exception as e:
            messagebox.showerror("Delete Error", f"Failed to delete record(s):\n{e}")

    def display_records(self, records):
        self.last_displayed_records = records
        self.output_box.delete("1.0", tk.END)

        if not records:
            self.output_box.insert(tk.END, "No records found.\n")
            return

        self.output_box.insert(tk.END, f"Found {len(records)} record(s)\n")
        self.output_box.insert(tk.END, "-" * 80 + "\n")

        if self.collapsed_view:
            for i, record in enumerate(records, start=1):
                animal_id = record.get("animal_id", "N/A")
                name = record.get("name", "N/A")
                animal_type = record.get("animal_type", "N/A")

                self.output_box.insert(
                    tk.END,
                    f"{i}. animal_id: {animal_id} | name: {name} | animal_type: {animal_type}\n"
                )

            self.output_box.insert(tk.END, "-" * 80 + "\n")
            self.output_box.insert(tk.END, "Collapsed view enabled.\n")
        else:
            for i, record in enumerate(records[:100], start=1):
                self.output_box.insert(tk.END, f"Record {i}:\n")
                for key, value in record.items():
                    self.output_box.insert(tk.END, f"  {key}: {value}\n")
                self.output_box.insert(tk.END, "-" * 80 + "\n")

            if len(records) > 100:
                self.output_box.insert(tk.END, f"\nOnly the first 100 records are displayed.\n")


    def toggle_collapse_view(self):
        self.collapsed_view = not self.collapsed_view

        if self.last_displayed_records:
            self.display_records(self.last_displayed_records)
        else:
            self.output_box.delete("1.0", tk.END)
            if self.collapsed_view:
                self.output_box.insert(tk.END, "Collapsed view is now enabled. Load or search for records to use it.\n")
            else:
                self.output_box.insert(tk.END, "Expanded view is now enabled. Load or search for records to use it.\n")

    def read_all_full(self):
        try:
            records = self.shelter.read({})
            self.display_records(records)
        except Exception as e:
            messagebox.showerror("Read Error", f"Failed to read records:\n{e}")

    def clear_output(self):
        self.output_box.delete("1.0", tk.END)
        self.last_displayed_records = []


if __name__ == "__main__":
    root = tk.Tk()
    app = AnimalShelterApp(root)
    root.mainloop()