using System;
using System.Net;
using System.Net.Mail;
using System.Net.Mime;
using System.Threading;
using System.ComponentModel;

namespace SMTP
{
    class Program
    {
        const string SERVER = "smtp.yandex.ru";
        const int PORT = 587;
        static void Main(string[] args)
        {
            var receiver = args[0];
            var sender = args[1];
            var pass = args[2];


            SmtpClient client = new SmtpClient(SERVER, PORT);
            MailAddress to = new MailAddress(receiver);
            MailAddress from = new MailAddress(sender);

            client.Credentials = new NetworkCredential(sender, pass);   
            client.EnableSsl = true;

            Console.WriteLine("\nInsert Subject.");
            string message_subject = Console.ReadLine();
            Console.WriteLine("\nInsert message.");
            string message_body = Console.ReadLine();
            Console.WriteLine("\nEnter an interval for sending in ms");
            int interval = 1;
            if (!int.TryParse(Console.ReadLine(), out interval))
            { 
                Console.WriteLine("You have entered an incorrect value.");
                return;
            }
            Console.WriteLine("\nEnter filename for attach. (Just press enter if dont want)");
            string file = Console.ReadLine();

            MailMessage message = new MailMessage(from, to);
            if (file.Length != 0)
            {
                var attachment = new Attachment(file);
                if (attachment != null) {
                    message.Attachments.Add(attachment);
                }
            }

            message.Body = message_body;
            message.Subject = message_subject;

            client.SendCompleted += new SendCompletedEventHandler(SendCompletedCallback);

            while (true) {
                client.SendAsync(message, "test");
                Console.WriteLine("\nSending message.");
                Thread.Sleep(interval);
            }
        }

        private static void SendCompletedCallback(object sender, AsyncCompletedEventArgs e)
        {
            if (e.Cancelled)
            {
                Console.WriteLine("Send canceled.");
            }
            if (e.Error != null)
            {
                Console.WriteLine("{0}", e.Error.ToString());
            }
            else
            {
                Console.WriteLine("Message sent.");
            }
        }
    }
}
