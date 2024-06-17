def calculate_average(values):
    # Split the input string by commas and convert each part to a float
    numbers = [float(value.strip()) for value in values.split(',')]
    
    # Calculate the sum of the numbers
    total_sum = sum(numbers)
    
    # Calculate the average
    average = total_sum / len(numbers) if numbers else 0
    
    return average

def main():
    # Prompt the user to enter values separated by commas
    user_input = input("Enter values separated by commas: ")
    
    try:
        # Calculate the average of the entered values
        average = calculate_average(user_input)
        
        # Print the average
        print(f"The average of the entered values is: {average}")
    except ValueError:
        print("Please enter valid numbers separated by commas.")

if __name__ == "__main__":
    main()
