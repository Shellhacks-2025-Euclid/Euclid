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
              <a href={"#"} rel={"noreferrer"}>MIT Licence</a>
            </li>
            <li role="listitem" className="text-sm hover:underline cursor-pointer">
              <a href={"#"} rel={"noreferrer"}>Another Licence</a>
            </li>
          </ul>
        </div>

        <div className="flex flex-col">
          <p className="text-muted-foreground text-sm">
            Authors
          </p>
          <ul className="flex flex-col gap-4 mt-4" role="list">
            <li role="listitem" className="text-sm hover:underline cursor-pointer">
              <a href={"#"} rel={"noreferrer"}>Niyaz Nassyrov</a>
            </li>
            <li role="listitem" className="text-sm hover:underline cursor-pointer">
              <a href={"#"} rel={"noreferrer"}>Nikita Muzychenko</a>
            </li>
            <li role="listitem" className="text-sm hover:underline cursor-pointer">
              <a href={"#"} rel={"noreferrer"}>Dmytro Parkhomenko</a>
            </li>
            <li role="listitem" className="text-sm hover:underline cursor-pointer">
              <a href={"#"} rel={"noreferrer"}>Furkan Tellioğlu</a>
            </li>
          </ul>
        </div>

        <div className="flex flex-col">
          <p className="text-muted-foreground text-sm">
            Project source code
          </p>
          <ul className="flex flex-col gap-4 mt-4" role="list">
            <li role="listitem" className="text-sm hover:underline cursor-pointer">
              <a href={"#"} rel={"noreferrer"}>Euclid</a>
            </li>
          </ul>
        </div>

        <div className="flex flex-col">
          <p className="text-muted-foreground text-sm">
            Contacts
          </p>
          <ul className="flex flex-col gap-4 mt-4" role="list">
            <li role="listitem" className="text-sm hover:underline cursor-pointer">
              <a href={"#"} rel={"noreferrer"}>Email</a>
            </li>
            <li role="listitem" className="text-sm hover:underline cursor-pointer">
              <a href={"#"} rel={"noreferrer"}>Telegram</a>
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