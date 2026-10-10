# Getting your free Västtrafik key

The display asks Västtrafik for live departures. Västtrafik requires every app to have its own **key** (a
Client ID and a Client secret). It is free. **Never share yours and never put it in a public place**, so this
project does not contain one: you enter yours on the display's setup page.

> These steps follow the developer portal and independent guides (for example Home Assistant's Västtrafik
> page). The portal can change its wording. If something looks different, look for the same ideas:
> *create an application*, *subscribe it to Planera Resa v4*, *get its key and secret*.

## Steps

1. Go to **[developer.vasttrafik.se](https://developer.vasttrafik.se)** and log in or create an account
   (use *Logga in* / register).
2. Open **Applications** (*Applikationer*, `developer.vasttrafik.se/applications`) and **create a new
   application**. Name it anything, for example "Bus Stop Display".
3. Open your application and **subscribe it to the API** called **Planera Resa v4**
   (button *Prenumerera på nytt API*). Creating the application alone is not enough: without the
   subscription Västtrafik answers with errors such as 403.
4. Find the application's **key and secret**. Copy both somewhere safe, for example a password manager.
   - the **key** is what the display calls **Client ID**
   - the **secret** is what the display calls **Client secret**
5. Wait a few minutes. Some people report that a new subscription needs up to about 15 minutes before it
   works.

## Using it

Do this **before** you start the display's setup (the setup page needs the key in step 1, and your phone has
no internet while it is connected to the display's own WiFi).

On the setup page, paste the key into **Client ID** and **Client secret**. The display stores them in its own
memory. Nothing is sent anywhere except to Västtrafik, to log in.

## If it does not work

- **"Västtrafik did not accept the key"** on the setup page: check that you copied both values completely
  (no spaces at the start or end), that the application is subscribed to **Planera Resa v4**, and wait a few
  minutes.
- **Departures never appear:** see [troubleshooting](troubleshooting.md).
- **Leaked your key by accident?** Create a new key (or a new application) in the portal and delete the old
  one, then enter the new one on the display (hold the screen for 4 seconds to open the setup page).

## Good to know

- This display only works with **Västtrafik**, so only stops in Västtrafik's area (Västra Götaland, for
  example Göteborg, Borås, Skövde, Trollhättan) can be chosen.
- Please read Västtrafik's terms for their API and follow them, including any limits on how often you may
  ask. The display asks for departures every 15 seconds.
