import Link from "next/link";

export default function Footer() {
  return (
    <footer className="w-full flex flex-col mt-[96px]">
      <div className="w-full py-[48px] flex justify-between gap-12 flex-col md:flex-row">
        <div className="flex flex-col">
          <p className="text-muted-foreground text-sm">
            Licences
          </p>
          <ul className="flex flex-col gap-4 mt-4" role="list">
            <li role="listitem" className="text-sm hover:underline cursor-pointer">
              <Link href={"/mit-licence"} rel={"noreferrer"}>MIT Licence</Link>
            </li>
            <li role="listitem" className="text-sm hover:underline cursor-pointer">
              <Link href={"/premake-licence"} rel={"noreferrer"}>Premake Licence</Link>
            </li>
          </ul>
        </div>

        <div className="flex flex-col">
          <p className="text-muted-foreground text-sm">
            Contributors
          </p>
          <ul className="flex flex-col gap-4 mt-4" role="list">
            <li role="listitem" className="text-sm flex gap-1">
              Niyaz Nassyrov •
              <a
                href={"https://www.linkedin.com/in/niyaz-nassyrov-b35205200"}
                rel={"noreferrer"}
                className="underline"
              >
                LinkedIn
              </a>
              •
              <a
                href={"https://github.com/nnniyaz"}
                rel={"noreferrer"}
                className="underline"
              >
                Github
              </a>
            </li>
            <li role="listitem" className="text-sm flex gap-1">
              Nikita Muzychenko •
              <a
                href={"https://www.linkedin.com/in/nikitamu/"}
                rel={"noreferrer"}
                className="underline"
              >
                LinkedIn
              </a>
              •
              <a
                href={"https://github.com/MuzychenkoNikita"}
                rel={"noreferrer"}
                className="underline"
              >
                Github
              </a>
            </li>
            <li role="listitem" className="text-sm flex gap-1">
              Dmytro Parkhomenko •
              <a
                href={"https://www.linkedin.com/in/dmytro-parkhomenko-71a46b387/"}
                rel={"noreferrer"}
                className="underline"
              >
                LinkedIn
              </a>
              •
              <a
                href={"https://github.com/LKAYHot"}
                rel={"noreferrer"}
                className="underline"
              >
                Github
              </a>
            </li>
            <li role="listitem" className="text-sm flex gap-1">
              Furkan Tellioğlu •
              <a
                href={"https://www.linkedin.com/in/furkan-emir-t-427b10251/"}
                rel={"noreferrer"}
                className="underline"
              >
                LinkedIn
              </a>
              •
              <a
                href={"https://github.com/FurkanT06"}
                rel={"noreferrer"}
                className="underline"
              >
                Github
              </a>
            </li>
          </ul>
        </div>

        <div className="flex flex-col">
          <p className="text-muted-foreground text-sm">
            Project source code
          </p>
          <ul className="flex flex-col gap-4 mt-4" role="list">
            <li role="listitem" className="text-sm hover:underline cursor-pointer">
              <a href={"https://github.com/Shellhacks-2025-Euclid/Euclid"} rel={"noreferrer"}>Euclid</a>
            </li>
          </ul>
        </div>

        <div className="flex flex-col">
          <p className="text-muted-foreground text-sm">
            Contacts
          </p>
          <ul className="flex flex-col gap-4 mt-4" role="list">
            <li role="listitem" className="text-sm hover:underline cursor-pointer">
              <a href={"mailto:nassyrovich@gmail.com"} rel={"noreferrer"}>Email</a>
            </li>
            <li role="listitem" className="text-sm hover:underline cursor-pointer">
              <a href={"https://t.me/niyaznassyrov"} rel={"noreferrer"}>Telegram</a>
            </li>
          </ul>
        </div>
      </div>

      <div className="w-full flex justify-center py-[24px]">
        <p className="text-muted-foreground text-sm">
          Euclid © 2025
        </p>
      </div>
    </footer>
  );
}