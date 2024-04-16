<?php
// Datos de conexión a la base de datos MySQL
$servername = "localhost";
$username = "root"; 
$password = ""; 
$dbname = "smc"; 

// Parámetros recibidos por POST
$peso = $_POST['peso'];
$talla = $_POST['talla'];
$temp_objeto = $_POST['temp_objeto'];

// Crear conexión a la base de datos
$conn = new mysqli($servername, $username, $password, $dbname);

// Verificar conexión
if ($conn->connect_error) {
  die("Error de conexión: " . $conn->connect_error);
}

// Preparar y ejecutar la consulta SQL para insertar los datos en la tabla
$sql = "INSERT INTO mediciones (peso, talla, temp_objeto) VALUES ('$peso', '$talla', '$temp_objeto')";

if ($conn->query($sql) === TRUE) {
  echo "Datos insertados correctamente";
} else {
  echo "Error al insertar datos: " . $conn->error;
}

// Cerrar conexión a la base de datos
$conn->close();
?>
