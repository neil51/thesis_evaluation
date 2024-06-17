import os
import csv
import glob

def read_csv_files(directory):
    # Get all CSV files in the specified directory
    csv_files = glob.glob(os.path.join(directory, "*.csv"))
    return csv_files

def calculate_average_third_column(csv_files):
    total_sum = 0
    count = 0
    
    for csv_file in csv_files:
        with open(csv_file, mode='r') as file:
            csv_reader = csv.reader(file)
            for row in csv_reader:
                # Convert the third column to a float and add it to the total sum
                total_sum += float(row[2])
                count += 1
                
    if count == 0:
        return 0
    
    return total_sum / count

def main():
    directory = 'astm_output/8_astm'  # Change this to the path where your CSV files are located
    csv_files = read_csv_files(directory)
    
    if not csv_files:
        print("No CSV files found in the directory.")
        return
    
    average = calculate_average_third_column(csv_files)
    print(f"The average of the third column is: {average}")

if __name__ == "__main__":
    main()
