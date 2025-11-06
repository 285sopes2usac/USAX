#include <usax/kernel/syscalls.h>
#include <usax/kernel/user.h>
#include <usax/kernel/errno.h>
#include <usax/kernel/kmalloc.h>

int sys_syslog(int type, char *bufp, int len)
{
    int bytes_leidos;
    char *buffer_kernel;

    // Por ahora, solo implementamos el comando para leer todo el buffer
   //  if (type != SYSLOG_ACTION_READ_ALL)
   //      return -EINVAL; // Error: Argumento inválido

    if (len < 0)
        return -EINVAL; // El tamaño no puede ser negativo

    // Es crucial verificar que el puntero del usuario es válido para escribir
   //  if (verify_user_wptr(bufp, len))
   //      return -EFAULT; // Error: Puntero inválido

    // Reservamos un buffer temporal en el kernel para evitar problemas de seguridad
    buffer_kernel = kmalloc(len);
    if (!buffer_kernel)
        return -ENOMEM; // Error: No hay suficiente memoria

    // 1. Llamamos a nuestra función interna para leer el ring buffer
    bytes_leidos = read_printk_ringbuf(buffer_kernel, len);

    // 2. Copiamos los datos de forma segura desde el kernel al espacio de usuario
    if (copy_to_user(bufp, buffer_kernel, bytes_leidos)) {
        kfree(buffer_kernel);
        return -EFAULT; // Error durante la copia
    }

    // Liberamos el buffer temporal y devolvemos el número de bytes leídos
    kfree(buffer_kernel);
    return bytes_leidos;
}
