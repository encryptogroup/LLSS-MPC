def generate_poly_activation_circuit(weighted_sum_node_id, start_node_id):
    """
    Generates a circuit description for the 6th-degree polynomial approximation
    of the sigmoid activation function.

    The circuit takes one input (the weighted sum from a neuron) and
    produces one output (the activated neuron value).

    Args:
        weighted_sum_node_id (int): The ID of the input node for the function.
        start_node_id (int): The starting node ID for the operations within this function.

    Returns:
        tuple: A tuple containing:
               - list: The generated circuit lines.
               - int: The ID of the final output node of the activation function.
    """
    node_id = start_node_id
    circuit_lines = []

    # --- Activation Function (Polynomial Approximation) ---
    # P_6(x) = 1/2 + 1/4*x - 1/48*x^3 + 1/480*x^5
    
    # Create nodes for the polynomial terms.
    # x^2, x^3, x^5
    x_squared_id = node_id
    circuit_lines.append(f"*:64,Scalar,Integer>64,Scalar,Integer < Plain:Plain > {{ Plain:Plain }} {weighted_sum_node_id} {weighted_sum_node_id} | {x_squared_id}")
    node_id += 1
    
    x_cubed_id = node_id
    circuit_lines.append(f"*:64,Scalar,Integer>64,Scalar,Integer < Plain:Plain > {{ Plain:Plain }} {x_squared_id} {weighted_sum_node_id} | {x_cubed_id}")
    node_id += 1
    
    # Note: We need x^4 first to get x^5
    x_fourth_id = node_id
    circuit_lines.append(f"*:64,Scalar,Integer>64,Scalar,Integer < Plain:Plain > {{ Plain:Plain }} {x_cubed_id} {weighted_sum_node_id} | {x_fourth_id}")
    node_id += 1

    x_fifth_id = node_id
    circuit_lines.append(f"*:64,Scalar,Integer>64,Scalar,Integer < Plain:Plain > {{ Plain:Plain }} {x_fourth_id} {weighted_sum_node_id} | {x_fifth_id}")
    node_id += 1
    
    # Create input nodes for the polynomial coefficients
    # The user can define these as inputs to be provided at execution
    c1_id = node_id
    circuit_lines.append(f"*:64,Scalar,Integer>64,Scalar,Integer < > {{ Plain:Plain }} {weighted_sum_node_id} c2 | {c1_id}") # Represents 1/2
    node_id += 1
    
    c2_id = node_id
    circuit_lines.append(f"*:64,Scalar,Integer>64,Scalar,Integer < > {{ Plain:Plain }} {x_cubed_id} c2 | {c2_id}") # Represents 1/4
    node_id += 1
    
    c3_id = node_id
    circuit_lines.append(f"*:64,Scalar,Integer>64,Scalar,Integer < > {{ Plain:Plain }} {x_fifth_id} c2 | {c3_id}") # Represents -1/48
    node_id += 1
    
    
    # Sum all the terms
    current_poly_sum = c1_id  # Start with the constant term
    circuit_lines.append(f"+:64,Scalar,Integer>64,Scalar,Integer < > {{ Plain:Plain }} c1 {c1_id} | {node_id}")
    current_poly_sum = node_id
    node_id += 1
    
    circuit_lines.append(f"+:64,Scalar,Integer>64,Scalar,Integer < > {{ Plain:Plain }} {current_poly_sum} {c2_id} | {node_id}")
    current_poly_sum = node_id
    node_id += 1
    
    circuit_lines.append(f"+:64,Scalar,Integer>64,Scalar,Integer < > {{ Plain:Plain }} {current_poly_sum} {c3_id} | {node_id}")
    current_poly_sum = node_id
    node_id += 1
    
    # The final output of the activation function
    neuron_output_id = current_poly_sum

    return circuit_lines, neuron_output_id, node_id

def generate_weighted_sum(last_layer_node_ids,start_node_id):
    circuit_lines = []
    counter = start_node_id
    assert(len(last_layer_node_ids) >= 2)

    factored_last_layer_ids = []

    for last_layer_node_id in last_layer_node_ids:
        circuit_lines.append(f"*:64,Scalar,Integer>64,Scalar,Integer < > {{ Plain:Plain }} c3 {last_layer_node_id} | {counter}")
        factored_last_layer_ids.append(counter)
        counter += 1



    circuit_lines.append(f"+:64,Scalar,Integer>64,Scalar,Integer < > {{ Plain:Plain }} {factored_last_layer_ids[0]} {factored_last_layer_ids[1]} | {counter}")
    counter += 1
    for factored_last_layer_id in factored_last_layer_ids[2:]:
        circuit_lines.append(f"+:64,Scalar,Integer>64,Scalar,Integer < > {{ Plain:Plain }} {counter - 1} {factored_last_layer_id} | {counter}")
        counter += 1
    return circuit_lines, counter


def generate_network_layer(last_layer_node_ids,layer_width,start_node_id):
    node_id = start_node_id
    circuit_lines = []
    output_node_ids = []
    for i in range(layer_width):
        sum_lines, node_id = generate_weighted_sum(last_layer_node_ids,node_id)
        node_id_sum = node_id - 1
        circuit_lines += sum_lines
        activation_lines, activation_node, node_id = generate_poly_activation_circuit(node_id_sum, node_id)
        output_node_ids.append(activation_node)
        circuit_lines += activation_lines
    return circuit_lines,output_node_ids , node_id

import sys

def generate_network():
    circuit_lines = []

    inputs = 784
    layers = [128,128,64]
    outputs = 10

    for i in range(inputs):
        circuit_lines.append(f"INPUT:64,Scalar,Integer>64,Scalar,Integer < Plain:Plain > {{ Plain:Plain }} | {i}")

    last_layer_node_ids = range(inputs)

    node_id = inputs

    for layer in layers:
        network_lines,last_layer_node_ids , node_id = generate_network_layer(last_layer_node_ids,layer,node_id)
        circuit_lines += network_lines
        
    
    for i in range(outputs):
        sum_lines, node_id = generate_weighted_sum(last_layer_node_ids,node_id)
        circuit_lines += sum_lines
        circuit_lines.append(f"OUTPUT:64,Scalar,Integer>64,Scalar,Integer < Plain:Plain > {{ Plain:Plain }} {node_id-1} | {node_id}")
        node_id += 1
    return circuit_lines



circuit_lines = generate_network()

with open("Circuits/NN.txt",'w') as file:
    for line in circuit_lines:
        file.write(line + "\n")
print("Created NN.txt")
