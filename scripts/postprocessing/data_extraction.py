def extract_data_corrected(data, method_name):
    extracted_data = []
    for entry in data:
        input_data = entry["input_data"]
        key_data = {
            "Dias": input_data["dias"],
            "Enfermeras": input_data["numero_enfermeras"],
            "Demanda": input_data["demanda"],
            "LS": input_data["limite_inferior_asignaciones"],
            "US": input_data["limite_superior_asignaciones"]
        }
        if "mensaje" in entry:
            extracted_data.append({**key_data, "FO": "X", "Tiempo": None})
        else:
            tiempo_exec = entry["tiempo_de_exec_segundos"]
            
            extracted_data.append({
                **key_data,
                "FO": entry["FO"],
                "Tiempo": tiempo_exec,
                "Nodos Explorados": entry["nodos_explorados"],
                "Metodo": method_name
            })
    return extracted_data