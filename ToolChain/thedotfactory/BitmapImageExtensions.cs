using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media.Imaging;

namespace TheDotFactory
{
    public static class BitmapImageExtensions
    {
        public static BitmapImage convertBitmapSourceToBitmapImage(BitmapSource bitmapSource)
        {
            if (bitmapSource == null) return null;
            
            //JpegBitmapEncoder encoder = new JpegBitmapEncoder();
            BmpBitmapEncoder encoder = new BmpBitmapEncoder();
            MemoryStream memoryStream = new MemoryStream();
            BitmapImage bImg = new BitmapImage();

            encoder.Frames.Add(BitmapFrame.Create(bitmapSource));
            encoder.Save(memoryStream);

            memoryStream.Position = 0;
            bImg.BeginInit();
            bImg.CacheOption = BitmapCacheOption.OnLoad;
            bImg.StreamSource = memoryStream;
            bImg.EndInit();

            memoryStream.Close();

            return bImg;
        }


        public static bool IsEqual(this BitmapImage image1, BitmapImage image2)
        {
            if (image1 == null || image2 == null)
            {
                return false;
            }

            if (!(image1.PixelWidth == image2.PixelWidth && image1.PixelHeight == image2.PixelHeight))
            {
                return false;
            }

            return image1.ToBytes().SequenceEqual(image2.ToBytes());
        }

        public static bool IsEqual(this BitmapSource image1, BitmapSource image2)
        {
            if (image1 == null || image2 == null)
            {
                return false;
            }

            if (!(image1.PixelWidth == image2.PixelWidth && image1.PixelHeight == image2.PixelHeight))
            {
                return false;
            }

            return image1.ToBytes().SequenceEqual(image2.ToBytes());
        }

        public static byte[] ToBytes(this BitmapSource bmpSrc)
        {
            var encoder = new BmpBitmapEncoder(); //JpegBitmapEncoder();
            //encoder.QualityLevel = 100;
            encoder.Frames.Add(BitmapFrame.Create(bmpSrc));

            using (var stream = new MemoryStream())
            {
                encoder.Save(stream);
                return stream.ToArray();
            }
        }

        public static byte[] ToBytes(this BitmapImage image)
        {
            byte[] data = new byte[] { };
            if (image != null)
            {
                try
                {
                    var encoder = new BmpBitmapEncoder();
                    encoder.Frames.Add(BitmapFrame.Create(image));
                    using (MemoryStream ms = new MemoryStream())
                    {
                        encoder.Save(ms);
                        data = ms.ToArray();
                    }
                    return data;
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.Message);
                }
            }
            return data;
        }
    }
}
